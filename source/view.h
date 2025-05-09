#pragma once

#include <string>

#include <SDL2/SDL.h>
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include "model.h"
#include "camera.h"

GLuint LoadShader(const std::string& shader_file_name, GLenum shader_type);
void CheckLinkStatus(GLuint& shader_program);

// Light structure definitions to match shader
struct DirectionalLight {
    glm::vec3 direction;
    glm::vec3 color;
    float intensity;
};

struct PointLight {
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 color;
    float intensity;
    float cutoff;
    float outer_cutoff;
    float constant;
    float linear;
    float quadratic;
};

class View {
private:
    // Legacy light uniform
    GLuint light_position_uniform_attribute;
    
    // Camera and material uniforms
    GLuint camera_position_uniform_attribute;
    GLuint texture_uniform_attribute;
    GLuint use_texture_uniform_attribute;
    GLuint material_shininess_uniform_attribute;
    GLuint material_specular_uniform_attribute;
    GLuint model_matrix_uniform_attribute;
    GLuint mvp_uniform_attribute;
    GLuint ambient_value_uniform_attribute; // Add uniform location for ambient_value
    
    // Light count uniform locations
    GLuint directional_light_count_uniform;
    GLuint point_light_count_uniform;
    GLuint spotlight_count_uniform;
    
    // Light array uniform locations
    std::vector<GLuint> directional_light_uniforms;
    std::vector<GLuint> point_light_uniforms;
    std::vector<GLuint> spotlight_uniforms;

    GLuint triangle_settings;
    GLuint triangle_buffer;
    GLuint color_buffer;
    GLuint shader_program;
    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint element_buffer;

    Model& model;
    
    // Set up uniform locations for lights
    void setupLightUniformLocations();
    
public:
    // Light definitions
    static const int MAX_DIRECTIONAL_LIGHTS = 4;
    static const int MAX_POINT_LIGHTS = 4;
    static const int MAX_SPOTLIGHTS = 4;
    
    // Light collections
    std::vector<DirectionalLight> directional_lights;
    std::vector<PointLight> point_lights;
    std::vector<SpotLight> spotlights;
    
    // Light helper methods
    void addDirectionalLight(const glm::vec3& direction, const glm::vec3& color = glm::vec3(1.0f), float intensity = 1.0f);
    void addPointLight(const glm::vec3& position, const glm::vec3& color = glm::vec3(1.0f), float intensity = 1.0f,
                      float constant = 1.0f, float linear = 0.09f, float quadratic = 0.032f);
    void addSpotLight(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& color = glm::vec3(1.0f),
                     float intensity = 1.0f, float cutoff = glm::cos(glm::radians(12.5f)), 
                     float outer_cutoff = glm::cos(glm::radians(17.5f)), float constant = 1.0f,
                     float linear = 0.09f, float quadratic = 0.032f);
    
    std::vector<float> background_color = { 0.0f,0.0f,0.0f,1.0f };
    //glm::vec3 light_position = glm::vec3(250.0f, 250.0f, 250.0f); // Legacy light position

    Camera& camera;
    View(Model& model, Camera& camera);
    void render();
    ~View();
    
private:
    class SceneLoader {
    private:
        View& view;
        std::vector<float> model_data_vector;
        void loadModel(const Model& model);
        void setupBuffers();
    public:
        GLuint number_of_vertices_in_scene;
        GLuint vertex_array_object;
        GLuint vertex_buffer;
        GLuint element_buffer;
        SceneLoader(View& view);
    };
    SceneLoader* scene_loader; 
};

