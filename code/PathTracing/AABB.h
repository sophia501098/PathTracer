#pragma once
#include<iostream>
#include<vector>
#include<array>
#include "Triangle.h"
#include "Ray.h"
#ifndef BBXEPSILON
#define BBXEPSILON 1e-3
#endif // !EPSILON
using namespace std;
class AABB {
public:
	glm::vec3 minBound;
	glm::vec3 maxBound;
	AABB(glm::vec3 min, glm::vec3 max) {
		minBound = min;
		maxBound = max;
	}
	AABB(const Triangle& tri) {
		minBound.x = std::min(std::min(tri.v0.x, tri.v1.x), tri.v2.x) - BBXEPSILON;
		maxBound.x = std::max(std::max(tri.v0.x, tri.v1.x), tri.v2.x) + BBXEPSILON;
		minBound.y = std::min(std::min(tri.v0.y, tri.v1.y), tri.v2.y) - BBXEPSILON;
		maxBound.y = std::max(std::max(tri.v0.y, tri.v1.y), tri.v2.y) + BBXEPSILON;
		minBound.z = std::min(std::min(tri.v0.z, tri.v1.z), tri.v2.z) - BBXEPSILON;
		maxBound.z = std::max(std::max(tri.v0.z, tri.v1.z), tri.v2.z) + BBXEPSILON;
	}
	AABB(){
		double minNum = std::numeric_limits<double>::lowest();
		double maxNum = std::numeric_limits<double>::max();
		minBound = glm::vec3(minNum, minNum, minNum);
		maxBound = glm::vec3(maxNum, maxNum, maxNum);
	}
	AABB(AABB& aabb) {
		minBound = aabb.minBound;
		maxBound = aabb.maxBound;
	}
	AABB operator =(const AABB& aabb) {
		minBound = aabb.minBound;
		maxBound = aabb.maxBound;
		return *this;
	}
	AABB(const AABB& a, const AABB& b) {
		minBound.x = std::min(a.minBound.x, b.minBound.x);
		minBound.y = std::min(a.minBound.y, b.minBound.y);
		minBound.z = std::min(a.minBound.z, b.minBound.z);

		maxBound.x = std::max(a.maxBound.x, b.maxBound.x);
		maxBound.y = std::max(a.maxBound.y, b.maxBound.y);
		maxBound.z = std::max(a.maxBound.z, b.maxBound.z);
	}
	bool IntersectAABB(const Ray& ray, const glm::vec3& invDir, const std::array<int, 3>& dirIsNeg) const
	{
		glm::vec3 min_t = (minBound - ray.ori) * invDir;
		glm::vec3 max_t = (maxBound - ray.ori) * invDir;
		if (dirIsNeg[0]) std::swap(min_t.x, max_t.x);
		if (dirIsNeg[1]) std::swap(min_t.y, max_t.y);
		if (dirIsNeg[2]) std::swap(min_t.z, max_t.z);
		float t_enter = std::max(std::max(min_t.x, min_t.y), min_t.z);
		float t_exit = std::min(std::min(max_t.x, max_t.y), max_t.z);
		if (t_enter <= t_exit && t_exit >= 0) 
			return true;
		else 
			return false;
	}
};