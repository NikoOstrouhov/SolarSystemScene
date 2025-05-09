#include "model.h"
#include "obj_loader.h" // Keep include for ObjLoader class (now with Assimp)
#include <iostream>

// Update the constructor to initialize material and ambient_value
Model::Model() :
    background_color(std::vector<double>{0, 0, 0, 1}), // Keep if needed
    color(1.0f, 1.0f, 1.0f), // Keep default color if used
    material(glm::vec3(1.0f, 1.0f, 1.0f)), // Initialize default material
    ambient_value(0.1f) // Initialize default ambient value
{
}

// Updated constructor taking model path, default color, and ambient value
Model::Model(const std::string& model_path, const glm::vec3& default_color, float ambient) :
    background_color(std::vector<double>{0, 0, 0, 1}), // Keep if needed
    color(default_color), // Use provided default color
    material(default_color), // Initialize material with default color
    ambient_value(ambient) // Initialize ambient value
{
    if (!loadModel(model_path)) { // Call the new loading function
        std::cerr << "Failed to load model from: " << model_path << std::endl;
        // Handle error appropriately
    }
    // Material loading is now handled within loadModel via Assimp
}

// Renamed and updated function to use Assimp loader
bool Model::loadModel(const std::string& path) {
    // Clear previous data if any
    vertices.clear();
    normals.clear();
    texture_coords.clear();
    indices.clear();
    // Reset material? Or rely on Assimp overwriting it? Let's clear it for safety.
    material = Material(); 

    // Call the static Assimp loading function from ObjLoader
    bool success = ObjLoader::loadModelWithAssimp(path, *this); 
    
    // Name setting is now handled inside loadModelWithAssimp
    
    // Material loading is also handled inside loadModelWithAssimp
    
    return success;
}

bool Model::hasTextureCoords() const {
    return !texture_coords.empty();
}

bool Model::hasTexture() const {
    // Check based on the loaded material data (e.g., if a texture path was loaded)
    return material.hasTexture(); 
}
