#pragma once
#include"Mesh.h"
class Intersection {
public:
	bool happened;
	glm::vec3 pos;
	glm::vec2 texCoords;
	glm::vec3 normal;
	glm::vec3 emit;
	double distance;
	Mesh* mesh;
	Intersection() {
		happened = false;
		pos = glm::vec3(0.0f);
		normal = glm::vec3(0.0f);
		texCoords = glm::vec2(0.0f);
		distance = std::numeric_limits<double>::max();
		mesh = nullptr;
	}
};
class Ray {
public:
	glm::vec3 ori;
	glm::vec3 dir;
	double t = 0;
	Ray(const glm::vec3& _ori,const glm::vec3& _dir) {
		ori = _ori;
		dir = _dir;
	}
	glm::vec3 getEnd(float _t) const {
		return ori + _t * dir;
	}
};