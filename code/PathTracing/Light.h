#include "Triangle.h"
#include "Ray.h"
#include "utility.h"
class Light {
public:
	vector<Triangle> lightTriangles;
	float lightArea = 0;

	void AddLight(Triangle light) {
		lightTriangles.push_back(light);
		lightArea += light.area;
	}
	void Sample(float& pdf, Intersection& inter) {
		pdf = 1.0f / lightTriangles.size();
		float p = Utility::get_random_float();
		float emit_area_sum = 0;
		for (int i = 0; i < lightTriangles.size(); ++i) {
			emit_area_sum += 1;
			if (emit_area_sum > p * lightTriangles.size()) {
				auto& curT = lightTriangles[i];
				float a = Utility::get_random_float();
				float b = Utility::get_random_float();
				float u = 1 - sqrt(a);
				float v = sqrt(a) * b;
				inter.pos = u * (curT.v1 - curT.v0) + v * (curT.v2 - curT.v0) + curT.v0;
				inter.normal = curT.normal;
				inter.emit = curT.mesh->material.radiance;
				if (glm::length(inter.emit - glm::vec3(0.0f, 0.0f, 0.0f)) < 0.01f) {
					cout << glm::to_string(inter.emit);
					cout << glm::to_string(curT.mesh->material.radiance);
					throw("emit is zero");
				}
				pdf *= 1.0f/lightTriangles[i].area;
				break;
			}
		}
	}
};