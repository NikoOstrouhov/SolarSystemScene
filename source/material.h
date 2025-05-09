#pragma once

#include <string>
#include <glm/glm.hpp>
#include <GL/glew.h>

// Class representing material properties for a model
class Material {
public:
    // Diffuse properties
    glm::vec3 diffuse_color;
    std::string diffuse_texture_path;
    GLuint diffuse_texture_id;
    
    // Ambient properties
    glm::vec3 ambient_color;
    
    // Specular properties
    glm::vec3 specular_color;
    float shininess;
    
    // Constructor with default values
    Material(const glm::vec3& diffuse = glm::vec3(1.0f, 1.0f, 1.0f),
             const glm::vec3& ambient = glm::vec3(0.2f, 0.2f, 0.2f),
             const glm::vec3& specular = glm::vec3(1.0f, 1.0f, 1.0f),
             float shininess = 32.0f);
    
    // Check if material has texture
    bool hasTexture() const;
    
    // Load texture from path
    bool loadTexture();
    
    // Free texture resources
    void freeTexture();
};