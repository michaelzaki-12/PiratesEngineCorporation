#pragma once
#include <iostream>
#include "glad.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
class Shader {

public:
	Shader();
	~Shader();
	void Init(const char* VertexShader, const char* FragmentShader);
	void use();
	unsigned int  ID;

	void setBool(const std::string& name, bool value) const;
	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float value) const;
	void setVec2(const std::string& name, float x, float y) const;
	void setVec3(const std::string& name, float x, float y, float z) const;
	void setVec4(const std::string& name, float x, float y, float z, float w) const;
	void setMat2(const std::string& name, const glm::mat2 mat) const;
	void setMat3(const std::string& name, const glm::mat3 mat) const;
	void setMat4(const std::string& name, const glm::mat4 mat) const;
private:
	void checkCompileErrors(unsigned int shader, std::string type);
};