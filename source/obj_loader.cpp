#include "obj_loader.h"
#include "model.h" // Include model.h for the Model class definition
#include "texture_loader.h" // Assuming texture_loader handles texture loading
#include <iostream>
#include <glm/gtc/type_ptr.hpp> // For glm::make_vec3

// --- Assimp Loading Implementation ---

bool ObjLoader::loadModelWithAssimp(const std::string& path, Model& model) {
    Assimp::Importer importer;
    // aiProcess_Triangulate: Ensures all faces are triangles.
    // aiProcess_GenSmoothNormals: Generates smooth normals if they are missing.
    // aiProcess_FlipUVs: Flips texture coordinates vertically (often needed for OpenGL).
    // aiProcess_JoinIdenticalVertices: Optimizes the mesh by joining identical vertices.
    const aiScene* scene = importer.ReadFile(path, 
                                              aiProcess_Triangulate | 
                                           //   aiProcess_GenSmoothNormals | 
                                           //   aiProcess_FlipUVs |
                                              aiProcess_JoinIdenticalVertices);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return false;
    }

    // Process Assimp's root node recursively
    processNode(scene->mRootNode, scene, model);

    // Extract filename for model name
    size_t lastSlash = path.find_last_of("/\\");
    size_t lastDot = path.find_last_of(".");
    if (lastSlash == std::string::npos) lastSlash = 0;
    else lastSlash++;
    
    if (lastDot == std::string::npos || lastDot <= lastSlash) {
        model.name = path.substr(lastSlash);
    } else {
        model.name = path.substr(lastSlash, lastDot - lastSlash);
    }

    return true;
}

void ObjLoader::processNode(aiNode* node, const aiScene* scene, Model& model) {
    // Process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh, scene, model);
    }
    // Then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene, model);
    }
}

void ObjLoader::processMesh(aiMesh* mesh, const aiScene* scene, Model& model) {
    // Reserve space to avoid reallocations
    size_t initial_vertex_index = model.vertices.size();
    model.vertices.reserve(model.vertices.size() + mesh->mNumVertices);
    model.normals.reserve(model.normals.size() + mesh->mNumVertices);
    if (mesh->mTextureCoords[0]) { // Does the mesh contain texture coordinates?
        model.texture_coords.reserve(model.texture_coords.size() + mesh->mNumVertices);
    }
    model.indices.reserve(model.indices.size() + mesh->mNumFaces * 3); // Assuming triangles

    // Process vertex positions, normals, and texture coordinates
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        model.vertices.push_back(glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));
        
        if (mesh->HasNormals()) {
            model.normals.push_back(glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z));
        } else {
            model.normals.push_back(glm::vec3(0.0f, 0.0f, 0.0f)); // Placeholder if no normals
        }

        if (mesh->mTextureCoords[0]) { // Check if texture coordinates exist
            model.texture_coords.push_back(glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y));
        } else {
            model.texture_coords.push_back(glm::vec2(0.0f, 0.0f)); // Placeholder if no UVs
        }
    }

    // Process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        // Assimp ensures faces are triangulated with aiProcess_Triangulate
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            model.indices.push_back(initial_vertex_index + face.mIndices[j]);
        }
    }

    // Process material
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        
        // Load diffuse textures
        loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", model);
        // Load specular textures (if needed)
        // loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", model);
        // Load normal maps (if needed)
        // loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal", model); 
        // Load height maps (if needed)
        // loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height", model);

        // Get material colors if no textures are present or as fallback
        aiColor3D color;
        if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
            model.material.diffuse_color = glm::vec3(color.r, color.g, color.b);
        } else {
             model.material.diffuse_color = glm::vec3(1.0f); // Default white
        }
        if (material->Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS) {
            model.material.ambient_color = glm::vec3(color.r, color.g, color.b);
        } else {
             model.material.ambient_color = model.material.diffuse_color * 0.2f; // Default ambient based on diffuse
        }
        if (material->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS) {
            model.material.specular_color = glm::vec3(color.r, color.g, color.b);
        } else {
             model.material.specular_color = glm::vec3(0.5f); // Default grey specular
        }
        float shininess;
        if (material->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
            model.material.shininess = shininess;
        } else {
            model.material.shininess = 32.0f; // Default shininess
        }
    }
}

void ObjLoader::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName, Model& model) {
    // For simplicity, we'll just load the first diffuse texture found.
    // A more robust implementation would handle multiple textures per type
    // and potentially store them differently in the Model/Material class.
    if (type == aiTextureType_DIFFUSE && mat->GetTextureCount(type) > 0) {
        aiString str;
        mat->GetTexture(type, 0, &str); // Get the first texture of this type
        
        // Assuming texture paths in the model file are relative to the model file itself.
        // Construct the full path.
        std::string texturePath = str.C_Str();
        // TODO: Need a robust way to get the directory of the model file.
        // For now, assume textures are in a 'textures' subdirectory relative to executable or a known path.
        // This part might need adjustment based on project structure and how paths are handled.
        
        // Example: Prepend a known textures directory if not an absolute path
        // This logic needs refinement based on where textures are actually stored.
        // std::string modelDirectory = /* logic to get model's directory */;
        // if (texturePath.find(":") == std::string::npos && texturePath[0] != '/') { // Basic check for relative path
        //     texturePath = modelDirectory + "/" + texturePath; 
        // }

        model.material.diffuse_texture_path = texturePath; // Store the path

        // Optionally, immediately load the texture using your texture loader
        // if (!model.material.diffuse_texture_path.empty()) {
        //     model.material.diffuse_texture_id = TextureLoader::loadTexture(model.material.diffuse_texture_path);
        //     if (model.material.diffuse_texture_id == 0) {
        //         std::cerr << "Warning: Failed to load texture: " << model.material.diffuse_texture_path << std::endl;
        //     }
        // }
    }
    // Add similar blocks for other texture types (specular, normal, etc.) if needed.
}