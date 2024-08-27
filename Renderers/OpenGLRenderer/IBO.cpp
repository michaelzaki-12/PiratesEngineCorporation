#include "IBO.h"

IBO::IBO()
{
}

void IBO::GenerateIBO()
{
	glGenBuffers(1, &ID);
}

void IBO::Bind(std::vector<unsigned int> indices) const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(float), indices.data(), GL_STATIC_DRAW);
}

void IBO::UnBind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void IBO::Delete() const
{
	glDeleteBuffers(1, &ID);
}
