#include "VBO.h"
VBO::VBO()
{
}

VBO::~VBO()
{
}

void VBO::GenerateVBO()
{
	glGenBuffers(1, &ID);
}

void VBO::Bind(std::vector<Vertex> data) const
{
	glBindBuffer(GL_ARRAY_BUFFER, ID);
	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(Vertex), data.data(), GL_STATIC_DRAW);
}

void VBO::UnBind() const
{
	glDeleteVertexArrays(1, &ID);
}
