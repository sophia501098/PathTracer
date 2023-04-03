#pragma once
#include<iostream>
#include<string>
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "Mesh.h"
#include "xmlLoader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
class Model {
private:
    string directory;
    vector<Texture> textures_loaded;
    unsigned int TextureFromFile(const char* path, const string& directory)
    {
        string filename = string(path);
        filename = directory + '/' + filename;

        int width, height, nrComponents;
        stbi_set_flip_vertically_on_load(true);
        float* data = stbi_loadf(filename.c_str(), &width, &height, &nrComponents, 0);
        TextureImg textureimg;
        if(data){
            textureimg.height = height;
            textureimg.width = width;
            textureimg.channels = nrComponents;
            unsigned int size = height * width * nrComponents;
            textureimg.imgdata.resize(size);
            std::copy(data, data + size, textureimg.imgdata.data());
            stbi_image_free(data);
        }
        else
        {
            throw std::runtime_error("Texture failed to load at path : " + string(path));
            stbi_image_free(data);
        }
        textureImgs.push_back(textureimg);
        unsigned int textureID = textureImgs.size() - 1;
        return textureID;
    }
public:
    vector<TextureImg> textureImgs;
    vector<Mesh> meshes;
    float minv = std::numeric_limits<float>::max();
    float maxv = -std::numeric_limits<float>::max();
    void processNode(vector<aiMesh*>& aimeshes, aiNode* node, const aiScene* scene) {
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            aimeshes.push_back(mesh);
        }
        // process the children node
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(aimeshes, node->mChildren[i], scene);
        }
    }
    
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
    {
        vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true;
                    break;
                }
            }
            if (!skip)
            {   // 如果纹理还没有被加载，则加载它
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture); // 添加到已加载的纹理中
            }
        }
        return textures;
    }
    
    void processMesh(vector<aiMesh*>& aimeshes, const aiScene* scene) {
        
        for (int i = 0; i < aimeshes.size(); ++i) {
            auto aimesh = aimeshes[i];
            vector<Vertex> vertices;
            vector<unsigned int> indices;
            vector<Texture> textures;
            Material meshMaterial;
            //process vertices
            for (unsigned int i = 0; i < aimesh->mNumVertices; i++)
            {
                Vertex vertex;
                glm::vec3 vector;
                vector.x = aimesh->mVertices[i].x;
                vector.y = aimesh->mVertices[i].y;
                vector.z = aimesh->mVertices[i].z;
                vertex.Position = vector;
                if (vector.x > maxv) maxv = vector.x;
                if (vector.y > maxv) maxv = vector.y;
                if (vector.z > maxv) maxv = vector.z;
                if (vector.x < minv) minv = vector.x;
                if (vector.y < minv) minv = vector.y;
                if (vector.z < minv) minv = vector.z;

                vector.x = aimesh->mNormals[i].x;
                vector.y = aimesh->mNormals[i].y;
                vector.z = aimesh->mNormals[i].z;
                vertex.Normal = vector;

                if (aimesh->mTextureCoords[0])
                {
                    glm::vec2 vec;
                    vec.x = aimesh->mTextureCoords[0][i].x;
                    vec.y = aimesh->mTextureCoords[0][i].y;
                    vertex.TexCoords = vec;
                }
                else vertex.TexCoords = glm::vec2(0.0f, 0.0f);
                vertices.push_back(vertex);
            }
            //process indices
            for (unsigned int i = 0; i < aimesh->mNumFaces; i++)
            {
                aiFace face = aimesh->mFaces[i];
                for (unsigned int j = 0; j < face.mNumIndices; j++)
                    indices.push_back(face.mIndices[j]);
            }
            //process material and texture 
            if (aimesh->mMaterialIndex >= 0)
            {
                //process material
                aiMaterial* material = scene->mMaterials[aimesh->mMaterialIndex];
                string name(material->GetName().C_Str());
                meshMaterial.SetName(name);
                aiColor3D color;
                material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
                glm::vec3 Kd = glm::vec3(color.r, color.g, color.b);
                material->Get(AI_MATKEY_COLOR_SPECULAR, color);
                glm::vec3 Ks = glm::vec3(color.r, color.g, color.b);
                material->Get(AI_MATKEY_COLOR_AMBIENT, color);
                glm::vec3 Tr = glm::vec3(color.r, color.g, color.b);
                //cout << Tr.x << " " << Tr.y << " " << Tr.z << endl;
                float Ni;
                material->Get(AI_MATKEY_REFRACTI, Ni);
                float Ns;
                material->Get(AI_MATKEY_SHININESS, Ns);
                meshMaterial.SetCoefficient(Kd, Ks, Tr, Ns, Ni);

                //process texture
                vector<Texture> diffuseMaps = loadMaterialTextures(material,
                    aiTextureType_DIFFUSE, "texture_diffuse");
                textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
                /*vector<Texture> specularMaps = loadMaterialTextures(material,
                    aiTextureType_SPECULAR, "texture_specular");
                textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());*/
                meshMaterial.SetTextures(diffuseMaps);
            }

            Mesh mesh(vertices, indices, textures, meshMaterial);
            meshes.push_back(mesh);
        }
    }

	Model(string path) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            throw std::runtime_error("ERROR::ASSIMP::" + string(importer.GetErrorString()));
            return;
        }
        directory = path.substr(0, path.find_last_of('/'));
        aiNode* node = scene->mRootNode;
        vector<aiMesh*> meshes;
        processNode(meshes, scene->mRootNode, scene);
        processMesh(meshes, scene);
	}

    void SetLight(const xml& xmlContent) {
        const auto& lights = xmlContent.lights;
        for (int i = 0; i < lights.size(); ++i) {
        //for (int i = 0; i < 1; ++i) {
            const auto& light = lights[i];
            string lightname = light.name;
            glm::vec3 radiance = light.radiance;
            for (int j = 0; j < meshes.size(); ++j) {
                if (meshes[j].material.name == lightname) {
                    meshes[j].material.SetRadiance(radiance);
                    meshes[j].isEmit = true;
                }

            }
        }
    }
};