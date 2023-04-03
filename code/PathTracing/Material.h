#pragma once
#include <string>
#include <vector>
#include <cmath>
#include "utility.h"
#include"Texture.h"
using namespace std;
class Material {
public:
	glm::vec3 Kd;
	glm::vec3 Ks;
	glm::vec3 Tr;
	float Ns;
	float Ni;
	bool hasEmit = false;
	glm::vec3 radiance;
	string name;
	vector<Texture> textures;
	Material() { ; }
	Material(glm::vec3 _Kd, glm::vec3 _Ks, glm::vec3 _Tr,float _Ns,float _Ni) {
		Kd = _Kd;
		Ks = _Ks;
		Tr = _Tr;
		Ns = _Ns;
		Ni = _Ni;
	}
	void SetCoefficient(glm::vec3 _Kd, glm::vec3 _Ks, glm::vec3 _Tr, float _Ns, float _Ni) {
		Kd = _Kd;
		Ks = _Ks;
		Tr = _Tr;
		Ns = _Ns;
		Ni = _Ni;
	}
	void SetName(string _name) {
		name = _name;
	}
	void SetTextures(vector<Texture> _textures) {
		textures = _textures;
	}
	void SetRadiance(glm::vec3 _radiance) {
		radiance = _radiance;
		hasEmit = true;
	}
	static float fresnel_schlick(float NoV, float eta) {
		float R0 = (1 - eta) / (1 + eta);
		R0 *= R0;
		return R0 + (1 - R0) * std::pow(1 - NoV, 5);
	}
	glm::vec3 BRDF(const glm::vec3 wi, glm::vec3 wo, glm::vec3 N, const vector<TextureImg>& textureimgs, glm::vec2 texcoord=glm::vec2(0.0f)) {
		glm::vec3 result = glm::vec3(0.0f);
		glm::vec3 _Kd;
		if (textures.size() != 0) {
			const auto& tex = textureimgs[textures[0].id];
			int x = std::min(int((texcoord.x - floor(texcoord.x)) * tex.width), tex.width - 1),
			y = std::min(int((texcoord.y - floor(texcoord.y)) * tex.height), tex.height - 1);
			assert(x >= 0 &&y>=0);
		
			_Kd.x = tex.imgdata[(y * tex.width + x) * 3];
			_Kd.y = tex.imgdata[(y * tex.width + x) * 3 + 1];
			_Kd.z = tex.imgdata[(y * tex.width + x) * 3 + 2];
			
		}
		else {
			_Kd = Kd;
		}
		if (glm::length(Tr-glm::vec3(1.0f)) > 1e-4) {
			return glm::vec3(0.0f); //refraction
		}
		//diffuse
		float cosalpha = glm::dot(N, wo);
		if (cosalpha > 0.0f) {
			result += _Kd / M_PI;
		}
		//specular
		glm::vec3 h = glm::normalize((wo + wi) * 0.5f);
		cosalpha = glm::dot(N, h);
		//cosalpha = glm::dot(glm::reflect(-wi, N), wo);
		if (glm::dot(N,wo) > 0) {
			result += Ks * pow(cosalpha, Ns) / 2.0f / M_PI * (Ns + 2);
		}
		return result;
	}
	glm::vec3 toWorld(const glm::vec3& a, const glm::vec3& N) {
		glm::vec3 B, C;
		if (std::fabs(N.x) > std::fabs(N.y)) {
			float invLen = 1.0f / std::sqrt(N.x * N.x + N.z * N.z);
			C = glm::vec3(N.z * invLen, 0.0f, -N.x * invLen);
		}
		else {
			float invLen = 1.0f / std::sqrt(N.y * N.y + N.z * N.z);
			C = glm::vec3(0.0f, N.z * invLen, -N.y * invLen);
		}
		B = glm::cross(C, N);
		return a.x * B + a.y * C + a.z * N;
	}
	glm::vec3 toLocal(const glm::vec3& a, const glm::vec3& N) {
		glm::vec3 B, C;
		if (std::fabs(N.x) > std::fabs(N.y)) {
			float invLen = 1.0f / std::sqrt(N.x * N.x + N.z * N.z);
			C = glm::vec3(N.z * invLen, 0.0f, -N.x * invLen);
		}
		else {
			float invLen = 1.0f / std::sqrt(N.y * N.y + N.z * N.z);
			C = glm::vec3(0.0f, N.z * invLen, -N.y * invLen);
		}
		B = glm::cross(C, N);
		return glm::vec3(glm::dot(a, B), glm::dot(a, C), glm::dot(a, N));
	}
	glm::vec3 Sample(glm::vec3 wi, glm::vec3 normal, float& pdf) {
		float e1 = Utility::get_random_float();
		float e2 = Utility::get_random_float();
		float x = cos(2 * M_PI * e2) * sqrt(e1);
		float y = sin(2 * M_PI * e2) * sqrt(e1);
		float z = sqrt(1 - e1);
		pdf = 1.0 / M_PI * z;
		glm::vec3 localRay = glm::vec3(x, y, z);
		return toWorld(localRay, normal);
		
	}
	glm::vec3 Sample_BRDF(glm::vec3 wi, glm::vec3 normal, glm::vec3& brdf,float& pdf, const vector<TextureImg>& textureimgs, glm::vec2 texcoord = glm::vec2(0.0f)) {
		float e = Utility::get_random_float();
		float e1 = Utility::get_random_float();
		float e2 = Utility::get_random_float();
		glm::vec3 _Kd;
		if (textures.size() != 0) {
			const auto& tex = textureimgs[textures[0].id];
			int x = std::min(int((texcoord.x - floor(texcoord.x)) * tex.width), tex.width - 1),
				y = std::min(int((texcoord.y - floor(texcoord.y)) * tex.height), tex.height - 1);
			_Kd.x = tex.imgdata[(y * tex.width + x) * 3];
			_Kd.y = tex.imgdata[(y * tex.width + x) * 3 + 1];
			_Kd.z = tex.imgdata[(y * tex.width + x) * 3 + 2];
		}
		else {
			_Kd = Kd;
		}
		if(glm::length(Ks)<1e-4 && glm::length(Tr-glm::vec3(1.0f))<1e-4 && glm::length(_Kd)>1e-4){ //´¿ diffuse
			float x = cos(2 * M_PI * e2) * sqrt(e1);
			float y = sin(2 * M_PI * e2) * sqrt(e1);
			float z = sqrt(1 - e1);
			brdf = _Kd * 1.0f / M_PI * z;
			glm::vec3 localRay = glm::vec3(x, y, z);
			pdf = 1.0 / M_PI * z;
			return toWorld(localRay, normal);
		}
		else if(glm::length(Ks) > 1e-4 && glm::length(Kd) > 1e-4){ 
			//diffuse+specular
			float e_d = glm::length(Kd) / (glm::length(Kd) + glm::length(Ks));
			float e_s = glm::length(Ks) / (glm::length(Kd) + glm::length(Ks));
			if (e < e_d) {
				
				float x = cos(2 * M_PI * e2) * sqrt(e1);
				float y = sin(2 * M_PI * e2) * sqrt(e1);
				float z = sqrt(1 - e1);
				
				glm::vec3 localRay = glm::vec3(x, y, z);
				glm::vec3 d_wo = toWorld(localRay, normal);

				glm::vec3 h = glm::normalize((d_wo + wi) * 0.5f);
				glm::vec3 wi_local = toLocal(wi, normal);

				//float cosalpha = glm::dot(glm::vec3(-wi_local.x, -wi_local.y, wi_local.z), localRay);
				float cosalpha = glm::dot(h, normal);
				brdf = (_Kd / M_PI + Ks * (Ns + 2) / 2.0f / M_PI * pow(cosalpha, Ns)) * z;

				float diffuse_pdf = 1.0 / M_PI * z;
				float specular_pdf = (Ns + 1) / 2.0f / M_PI * pow(cosalpha, Ns);
				pdf = e_d * diffuse_pdf + e_s * specular_pdf;
				return d_wo;
			}
			else {
				
				float x = cos(2 * M_PI * e2) * sqrt(1 - pow(e1, 2 / (Ns + 1)));
				float y = sin(2 * M_PI * e2) * sqrt(1 - pow(e1, 2 / (Ns + 1)));
				float z = pow(e1, 1 / (Ns + 1));
				
				glm::vec3 localRay = glm::vec3(x, y, z);
				glm::vec3 s_wo_local = toWorld(localRay, glm::vec3(-wi.x, -wi.y, wi.z));
				glm::vec3 s_wo = toWorld(s_wo_local, normal);
				
				if (s_wo_local.z <= 1e-3) {
					brdf = glm::vec3(0.0f);
					pdf = 1.0f;
					return s_wo;
				}

				glm::vec3 h = glm::normalize((s_wo + wi) * 0.5f);
				glm::vec3 wi_local = toLocal(wi, normal);

				//float cosalpha = glm::dot(glm::vec3(-wi_local.x, -wi_local.y, wi_local.z), s_wo_local);
				//float cosalpha = glm::dot(glm::reflect(-wi, normal), s_wo);
				float cosalpha = glm::dot(h, normal);
				brdf = (_Kd / M_PI + Ks * (Ns + 2) / 2.0f / M_PI * pow(cosalpha, Ns)) * s_wo_local.z;
				
				float diffuse_pdf = 1.0 / M_PI * z;
				float specular_pdf = (Ns + 1) / 2.0f / M_PI * pow(cosalpha, Ns);
				pdf = e_d * diffuse_pdf + e_s * specular_pdf;
				return s_wo;
			}
		}
		else if (glm::length(_Kd) > 1e-4 && glm::length(Tr - glm::vec3(1.0f)) > 1e-4) {
			float NoV = glm::dot(normal, wi);
			bool back_face = NoV < 0;
			float eta = back_face ? Ni : 1 / Ni;
			auto const& Normal = back_face ? -normal : normal;

			float cos_theta = dot(wi, Normal);
			float sin_theta = std::sqrt(1 - cos_theta * cos_theta);

			bool cannot_refract = eta * sin_theta > 1.f;
			pdf = 1.f;
				
			if (cannot_refract || Utility::get_random_float() < fresnel_schlick(cos_theta, eta)) {
				brdf = _Kd;
				return glm::normalize(glm::reflect(-wi, Normal));
			}
			else {
				brdf = Tr;
				return glm::normalize(glm::refract(-wi, Normal, eta));
			}
		}
		//else if (glm::length(_Kd) > 1e-4 && glm::length(Tr-glm::vec3(1.0f)) > 1e-4) { 
		//	//diffuse + refraction
		//	float diffuse_pbf = glm::length(_Kd) / (glm::length(_Kd) + glm::length(Tr));
		//	float refraction_pbf = glm::length(Tr) / (glm::length(_Kd) + glm::length(Tr));
		//	if (e < diffuse_pbf && glm::dot(wi,normal)>0) {
		//		pdf = diffuse_pbf;
		//		brdf = _Kd;
		//		glm::vec3 localRay = toLocal(wi, normal);
		//		return toWorld(glm::vec3(-localRay.x, -localRay.y, localRay.z), normal);
		//	}
		//	else {
		//		
		//		float NoV = glm::dot(normal, wi);
		//		bool back_face = NoV < 0;
		//		float eta = back_face ? Ni : 1 / Ni;
		//		auto const& Normal = back_face ? -normal : normal;

		//		float cos_theta = dot(wi, Normal);
		//		float sin_theta = std::sqrt(1 - cos_theta * cos_theta);

		//		bool cannot_refract = eta * sin_theta > 1.f;
		//		

		//		if (cannot_refract || Utility::get_random_float() < fresnel_schlick(cos_theta, eta)) {
		//			brdf = _Kd;
		//			pdf = 1.f;
		//			return glm::normalize(glm::reflect(-wi, Normal));
		//		}
		//		else {
		//			pdf = refraction_pbf;
		//			brdf = Tr;
		//			return glm::normalize(glm::refract(-wi, Normal, eta));
		//		}
		//	}
		//}
		else {
			cout << "wrong" << endl;
		}
	}
};