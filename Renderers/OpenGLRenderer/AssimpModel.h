#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "Shader.h"
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include "../PCH.hpp"
class AssimpModel
{
public:
    AssimpModel(std::string path);
    void Draw(Shader& shader);

    // model data
    std::vector<Mesh> meshes;
    std::string directory;
    std::vector<Texture> textures_loaded;
private:

    unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma);
    void loadModel(std::string path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type,
        std::string typeName);
};
