#pragma once

#include <glad.h>
#include <string>
#include <iostream>
#include "../stb_image.h"
#include <vector>
class Texture {
public:
	Texture();
	~Texture();
	void Bind(int index);
	void BindCubeMap(int index);
	void UnBind();
	void GenerateTexture(int index, int param, bool gamma);
	void GenerateTexturefromFrameBuffer(int index, int param);
	void LoadTexture(std::string path_of_texture);
	void CubeMap(std::vector<std::string> faces, int index);
	unsigned int loadCubemap(std::vector<std::string> faces);
	unsigned int loadHDREquiRectangularMap(std::vector<std::string> hdrimage);
	void FreeTexture();
	std::vector<unsigned int> ID = std::vector<unsigned int>(16);
	unsigned int id;
	std::string type;
	std::string path;
	unsigned char* data;
	GLenum format{}, internalformat{};
	int width, height, nrChannels;

};