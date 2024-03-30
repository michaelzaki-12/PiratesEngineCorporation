#pragma once
#include <iostream>
#include "glad.h"
class Shader {

public:
	Shader(const char* VertexShader, const char* FragmentShader);
	~Shader();

	void use();
	unsigned int  ID;
private:
	void checkCompileErrors(unsigned int shader, std::string type);
};