#pragma once
#include<iostream>
#include<vector>
#include<string>
#include"svpng.h"
#include"Scene.h"
#include "tinyexr.h"
#ifndef EPSILON
#define EPSILON 1e-5
#endif // !EPSILON
using namespace std;
class Render {
private:
	uint32_t width;
	uint32_t height;
	uint32_t spp;
	vector<glm::vec3> framebuffer;
public:
	Render(uint32_t _w, uint32_t _h, uint32_t _spp) {
		width = _w;
		height = _h;
		spp = _spp;
		framebuffer.resize(width * height);
	}
	void Rendering(Scene& scene) {
		float imageAspectRatio = width * 1.0f / height;
		float scale = tan(Utility::deg2rad(scene.fov * 0.5));
		int m = 0;
		//const float gamma = 2.2;
		for (int j = 0; j < height; ++j) {
			#pragma omp parallel for
			for (int i = 0; i < width; ++i) {
				float x = ((i + 0.5) / width * 2 - 1) * imageAspectRatio * scale;
				float y = (1 - (j + 0.5) / height * 2) * scale;
				glm::vec4 dir(x, y, -1, 0);
				dir = scene.camera2World * dir;
				glm::vec3 raydir(dir.x, dir.y, dir.z);
				Ray ray(scene.cameraPos, raydir);
				
				for (int k = 0; k < spp; ++k) {
					framebuffer[j * width + i] += scene.castRay(ray, 0);
				}
				framebuffer[j * width + i] *= (1.0f / spp);
				
				//framebuffer[j * width + i] = framebuffer[j * width + i] / (framebuffer[j * width + i] + glm::vec3(1.0f));
				// Gamma校正
				//framebuffer[j * width + i] = glm::pow(framebuffer[j * width + i], glm::vec3(1.0 / gamma));
				
			}
			Utility::UpdateProgress(j / (float)height);
		}
		Utility::UpdateProgress(1.f);
	}
	void SaveExr(string filepath) {
		SaveEXR(&(framebuffer[0].x), width, height, 3, 0, filepath.c_str(), NULL);
	}
	void SavePNG(string filepath) {
		// save framebuffer to file
		const float gamma = 2.2;
		std::vector<unsigned char> rgb(height * width * 3);
		FILE* fp = fopen(filepath.c_str(), "wb");
		for (auto i = 0; i < height * width; ++i) {
			static unsigned char color[3];
			framebuffer[i] = framebuffer[i] / (framebuffer[i] + glm::vec3(1.0f)); // Reinhard色调映射
			framebuffer[i] = glm::pow(framebuffer[i], glm::vec3(1.0 / gamma));
			rgb[i * 3] = (unsigned char)(255 * Utility::clamp(0, 1, framebuffer[i].x));
			rgb[i * 3 + 1] = (unsigned char)(255 * Utility::clamp(0, 1, framebuffer[i].y));
			rgb[i * 3 + 2] = (unsigned char)(255 * Utility::clamp(0, 1, framebuffer[i].z));
		}
		svpng(fp, width, height, rgb.data(), 0);
		fclose(fp);
	}
};