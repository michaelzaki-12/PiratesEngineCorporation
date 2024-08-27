#pragma once

#include "glad.h"
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#define MAX_BONE_INFLUENCE 4
struct Vertex {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoord;
    // tangent
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;
    //bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    //weights from each bone
    float m_Weights[MAX_BONE_INFLUENCE];
};

class VBO {

public:
	VBO();
	~VBO();
    void GenerateVBO();
	void Bind(std::vector<Vertex> data) const;
	void UnBind() const;
	GLuint ID;

private:

};