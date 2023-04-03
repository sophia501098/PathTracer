#pragma once
#include<vector>
#include<string>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/string_cast.hpp"
using namespace std;
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};
struct Texture {
    unsigned int id;
    string type;
    string path;
};
class TextureImg {
public:
    vector<float> imgdata;
    int width;
    int height;
    int channels;
};
