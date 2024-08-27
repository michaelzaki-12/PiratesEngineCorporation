#include "VAO.h"

VAO::VAO()
{
}

void VAO::GenerateVAO()
{
	glGenVertexArrays(1, &ID);
}

VAO::~VAO()
{
}

void VAO::Bind() const
{
	glBindVertexArray(ID);
}

void VAO::UnBind() const
{
	glBindVertexArray(0);
}
