#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Forward declaration
class Model;

class ObjLoader {
public:
    // Updated function to load model using Assimp
    static bool loadModelWithAssimp(const std::string& path, Model& model);

private:
    // Helper function to process Assimp nodes recursively
    static void processNode(aiNode* node, const aiScene* scene, Model& model);
    // Helper function to process Assimp meshes
    static void processMesh(aiMesh* mesh, const aiScene* scene, Model& model);
    // Helper function to load materials (textures)
    static void loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName, Model& model);
};