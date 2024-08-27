#pragma once

#include "glad.h"
#include <iostream>

class VAO {
public:
	VAO();
	void GenerateVAO();
	virtual ~VAO();
	void Bind() const;
	void UnBind() const;
	GLuint ID;
private:

};