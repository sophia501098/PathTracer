#pragma once
#include"Mesh.h"
#include"utility.h"
#include"Ray.h"
#include"Morton64.h"
#ifndef EPSILON
    #define EPSILON 1e-4
#endif // !EPSILON

class Triangle {
public:
    unsigned long long mortonCode;
	glm::vec3 v0, v1, v2;
    glm::vec2 vt0, vt1, vt2;
	glm::vec3 e1, e2;
	glm::vec3 normal;
	float area;
	Mesh* mesh;
    
	Triangle(Vertex p0, Vertex p1, Vertex p2, Mesh* _mesh, float vmin, float vmax)
	{
        mesh = _mesh;
        v0 = p0.Position;
        v1 = p1.Position;
        v2 = p2.Position;
        vt0 = p0.TexCoords;
        vt1 = p1.TexCoords;
        vt2 = p2.TexCoords;
		e1 = v1 - v0;
		e2 = v2 - v0;
		normal = normalize((p0.Normal+p1.Normal+p2.Normal)/3.0f);
		area = glm::length(glm::cross(e1, e2)) * 0.5f;
        glm::vec3 center = (1.0f / 3 * (v0 + v1 + v2) - vmin) / (vmax - vmin);
       // std::cout << glm::to_string(center) << std::endl;
        Morton64 mortonEncoder;
        mortonEncoder.Encode(center.x, center.y, center.z);
        mortonCode = mortonEncoder.m_Value;
        //cout << mortonCode << endl;
	}
	bool IsEmit() {
		return mesh->isEmit;
	}

    

	//test if a ray intersect this triangle
	Intersection Intersect(const Ray& ray) {
		Intersection inter;

        /*if (glm::dot(ray.dir, normal) > 0)
            return inter;*/
        double u, v, t_tmp = 0;
        glm::vec3 pvec = glm::cross(ray.dir, e2);
        double det = glm::dot(e1, pvec);
        if (fabs(det) < EPSILON)
            return inter;

        double det_inv = 1. / det;
        glm::vec3 tvec = ray.ori - v0;
        u = glm::dot(tvec, pvec) * det_inv;
        if (u < 0 || u > 1)
            return inter;
        glm::vec3 qvec = glm::cross(tvec, e1);
        v = glm::dot(ray.dir, qvec) * det_inv;
        if (v < 0 || u + v > 1)
            return inter;
        t_tmp = glm::dot(e2, qvec) * det_inv;
        if (t_tmp < 1e-2)
           return inter;
        // find ray triangle intersection
        inter.happened = true;
        inter.normal = normal;
        inter.mesh = mesh;
        inter.distance = t_tmp;
        inter.pos = ray.getEnd(t_tmp);
        inter.texCoords = float(u) * vt1 + float(v) * vt2 + (1 - float(u) - float(v)) * vt0;
        if (IsEmit()) {
            inter.emit = mesh->material.radiance;
        }
        return inter;
	}
};
