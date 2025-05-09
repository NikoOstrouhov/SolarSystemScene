#pragma once
#include <vector>
#include <string>
#include <memory>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "material.h"

class Model {
public:
    glm::mat4 matrix = glm::identity<glm::mat4>();
    // Model data
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texture_coords;
    std::vector<unsigned int> indices;
    glm::vec3 color;
    std::string name;
    
    // Material for the model
    Material material;

    // Background color (for scene settings)
    std::vector<double> background_color;

    // Ambient value for the model
    float ambient_value;

    // Default constructor
    Model();
    
    // Constructor with path to .obj file
    Model(const std::string& obj_path, const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f), float ambient = 0.1f);
    
    // Load model from .obj file
    bool loadModel(const std::string& path);
    
    // Check if model has texture coordinates
    bool hasTextureCoords() const;
    
    // Check if model has material with texture
    bool hasTexture() const;
};

