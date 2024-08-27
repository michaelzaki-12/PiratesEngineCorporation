#pragma once

#include <iostream>
#include <string>
#include "../json.h"
#include "../Renderers/OpenGLRenderer/Shader.h"
#include <vector>
#include <filesystem>
#include <fstream>



class Model {
public:
	Model(std::string filepath);
	void Draw(Shader& shader);

private:
	std::string file;
	std::vector<unsigned char> data;
	nlohmann::json Json;
	std::vector<unsigned char> getdata();

	std::vector<float> GetFloats(nlohmann::json accessor);
	std::vector<float> GetIndices(nlohmann::json accessor);

};