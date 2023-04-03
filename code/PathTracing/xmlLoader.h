#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "pugixml.hpp"
#include<iostream>
#include<sstream>
using namespace std;
struct light {
	string name;
	glm::vec3 radiance;
};
struct xml {
	vector<light> lights;
	int width;
	int height;
	float fov;
	glm::vec3 cameraPos;
	glm::vec3 cameraUp;
	glm::vec3 cameraFront;
};
class xmlLoader {
public:
	xml readXML(string path) {
		xml xmlRes;
		pugi::xml_document doc;
		pugi::xml_parse_result result = doc.load_file(path.c_str());
		if (!result)
			throw std::runtime_error("fail to read xml file");
		auto camera = doc.child("camera");
		xmlRes.width = atof(camera.attribute("width").value());
		xmlRes.height = atof(camera.attribute("height").value());
		xmlRes.fov = atof(camera.attribute("fovy").value());
		std::cout << "camera width: " << xmlRes.width << std::endl;
		std::cout << "camera height: " << xmlRes.height << std::endl;
		std::cout << "camera fovy: " << xmlRes.fov << std::endl;

		float x, y, z;
		auto eye = camera.child("eye");
		x = atof(eye.attribute("x").value());
		y = atof(eye.attribute("y").value());
		z = atof(eye.attribute("z").value());
		std::cout << "eye:  (" << x << ", " << y << ", " << z << ")" << std::endl;
		xmlRes.cameraPos = glm::vec3(x, y, z);

		auto lookat = camera.child("lookat");
		x = atof(lookat.attribute("x").value());
		y = atof(lookat.attribute("y").value());
		z = atof(lookat.attribute("z").value());
		std::cout << "lookat:  (" << x << ", " << y << ", " << z << ")" << std::endl;
		xmlRes.cameraFront = glm::vec3(x, y, z);

		auto up = camera.child("up");
		x = atof(up.attribute("x").value());
		y = atof(up.attribute("y").value());
		z = atof(up.attribute("z").value());
		std::cout << "up:  (" << x << ", " << y << ", " << z << ")" << std::endl;
		xmlRes.cameraUp = glm::vec3(x, y, z);

		auto lights = doc.children("light");

		for (auto it = lights.begin(); it != lights.end(); ++it)
		{
			light curlight;
			curlight.name = it->attribute("mtlname").value();
			cout << curlight.name << " ";
			stringstream radiance(it->attribute("radiance").value());
			string temp;
			vector<float> res;
			while (getline(radiance, temp, ','))
			{
				res.push_back(atof(temp.c_str()));
			}
			x = res[0];
			y = res[1];
			z = res[2];
			std::cout << "radiance:  (" << x << ", " << y << ", " << z << ")" << std::endl;
			curlight.radiance = glm::vec3(x, y, z);
			xmlRes.lights.push_back(curlight);
		}
		return xmlRes;
	}
};