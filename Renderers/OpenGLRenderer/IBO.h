#pragma once
#include <iostream>
#include <glad.h>
#include <vector>

class IBO {
public:
	IBO();
	void GenerateIBO();
	void Bind(std::vector<unsigned int> indices) const;
	void UnBind() const;
	void Delete() const;
	unsigned int ID;
};