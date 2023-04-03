#pragma once
#include <vector>
#include <algorithm>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/string_cast.hpp"
#include "utility.h"
#include "Light.h"
#include "Ray.h"
#include "Model.h"
#include "BVH.h"
using namespace std;

class Scene {
public:
	float RussianRoulette = 0.8f;
	// camera
	float fov;
	glm::vec3 cameraPos;
	glm::mat4 camera2World;

	//light
	Light lights;

	// model
	std::unique_ptr<Model> model;
	vector<Triangle> triangles;
	
	// BVH
	BVH BVHTree;

	void Add(std::unique_ptr<Model> obj) {
		model = std::move(obj);
		for (int i = 0; i < model->meshes.size(); ++i) {
			auto& mesh = model->meshes[i];
			for (int j = 0; j < mesh.indices.size() / 3; ++j) {
				unsigned int pi0 = mesh.indices[j * 3 + 0];
				unsigned int pi1 = mesh.indices[j * 3 + 1];
				unsigned int pi2 = mesh.indices[j * 3 + 2];
				Vertex p0 = mesh.vertices[pi0];
				Vertex p1 = mesh.vertices[pi1];
				Vertex p2 = mesh.vertices[pi2];
				Triangle triangle(p0, p1, p2, &model->meshes[i], model->minv, model->maxv);
				triangles.push_back(triangle);
				if (mesh.isEmit == true) {
					lights.AddLight(triangle);
				}
			}
		}
		std::sort(triangles.begin(), triangles.end(), [](Triangle a, Triangle b) {
			return b.mortonCode > a.mortonCode;
		});
		/*cout << "sorted:" << endl;
		for (int i = 0; i < triangles.size(); ++i) {
			cout << triangles[i].mortonCode << endl;
		}*/
	}

	void BuildBVH() {
		BVHTree.BuildTree(triangles);
	}

	void SetCamera(glm::vec3 _cameraPos, glm::vec3 _cameraUp, glm::vec3 _cameraLookat, float _fov = 90.0f) {
		fov = _fov;
		cameraPos = _cameraPos;
		glm::mat4 RotateMatrix = glm::lookAt(_cameraPos, _cameraLookat, _cameraUp);
		RotateMatrix = glm::translate(RotateMatrix, cameraPos);
		camera2World = glm::inverse(RotateMatrix);
	}

	void SampleLight(float& pdf, Intersection& inter) {
		lights.Sample(pdf, inter);
	}

	Intersection GetIntersection(const Ray& ray) {
		return BVHTree.IntersectBVH(ray, triangles);
	}
	Intersection GetIntersection_iter(const Ray& ray) {
		Intersection inter;
		for (int i = 0; i < triangles.size(); ++i) {
			auto& curtri = triangles[i];
			Intersection curinter=curtri.Intersect(ray);
			
			if (curinter.happened && curinter.distance < inter.distance) {
				//cout <<"iter "<< i << ": " << curinter.happened << " ";
				inter = curinter;
			}
		}
		return inter;
	}
	glm::vec3 castRay(const Ray& ray, int depth){
		glm::vec3 dirL = glm::vec3(1.0f) * float(EPSILON);
		glm::vec3 indirL = glm::vec3(0.0f);
		/*Intersection pinter = GetIntersection_iter(ray);
		if (pinter.happened == true) 
			cout << endl;*/
		Intersection pinter = GetIntersection(ray);
		/*if (pinter.happened == true) 
			cout << endl;*/

		if (pinter.happened == false) 
			return glm::vec3(0.0f);
		if (pinter.mesh->isEmit) 
			return pinter.emit;

		glm::vec3 p = pinter.pos;
		glm::vec3 N = pinter.normal;
		
		// direct illumination
		float pdf_light;
		Intersection xinter;
		SampleLight(pdf_light, xinter);
		glm::vec3 x = xinter.pos;
		glm::vec3 wo = -ray.dir;
		glm::vec3 ws = glm::normalize(x - p);
		glm::vec3 NN = xinter.normal;
		if (glm::dot(-ws, NN) >= 1e-6) {
			Ray px_ray(p, glm::normalize(x - p));
			Intersection px_inter = GetIntersection(px_ray);
			if (px_inter.distance > glm::length(x - p) - 1e-3) {
				
				dirL = xinter.emit * pinter.mesh->material.BRDF(wo, ws, N, model->textureImgs, pinter.texCoords) * glm::dot(ws, N) * glm::dot(-ws, NN) / pdf_light / dot(x - p, x - p);

			}
		}
		if (Utility::get_random_float() < RussianRoulette) {
			float pdf;
			glm::vec3 brdf;
			ws = pinter.mesh->material.Sample_BRDF(wo, N, brdf, pdf, model->textureImgs, pinter.texCoords);
			
			Ray ray_pout(p, ws);
			Intersection poutinter = GetIntersection(ray_pout);
			if (poutinter.happened && !poutinter.mesh->isEmit) {
				glm::vec3 shaingColor = castRay(ray_pout, depth + 1);
				if (abs(pdf) > EPSILON && !isnan(brdf.x) && !isnan(brdf.y) && !isnan(brdf.z)) {
					indirL = shaingColor * brdf / RussianRoulette / pdf/1.25f;
				}
			}
		}
		
		return dirL + indirL;
	}
};