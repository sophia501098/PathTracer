#pragma once
#include"Texture.h"
#include"Material.h"
using namespace std;

class Mesh {
public:
    bool isEmit = false;
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;
    Material material;
    Mesh(vector<Vertex> _vertices, vector<unsigned int> _indices, vector<Texture> _textures, Material _material) {
        vertices = _vertices;
        indices = _indices;
        textures = _textures;
        material = _material;
    }
};