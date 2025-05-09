#include <fstream>
#include <iostream>

#include "view.h"

GLuint LoadShader(const std::string& shader_file_name, GLenum shader_type)
{
	std::ifstream my_stream(shader_file_name);
	std::string contents = std::string((std::istreambuf_iterator<char>(my_stream)), std::istreambuf_iterator<char>());
	const GLchar* shader_code = contents.c_str();

	GLuint shader = glCreateShader(shader_type);
	glShaderSource(shader, 1, &shader_code, nullptr);
	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE)
	{
		std::cout << "shader compile error in: " << shader_file_name << std::endl;
		char buffer[512];
		glGetShaderInfoLog(shader, 512, nullptr, buffer);
		std::cout << buffer << std::endl;
	}
	return shader;
}

void CheckLinkStatus(GLuint& shader_program)
{
	GLint link_status;
	glGetProgramiv(shader_program, GL_LINK_STATUS, &link_status);
	if (link_status != GL_TRUE)
	{
		std::cout << "shader linker error in View constructor" << std::endl;
		char buffer[512];
		glGetProgramInfoLog(shader_program, 512, nullptr, buffer);
		std::cout << buffer << std::endl;
	}
}

View::View(Model& model, Camera& camera) : model(model), camera(camera) {
	glEnable(GL_DEPTH_TEST);

	vertex_shader = LoadShader(std::string("./shaders/vertex.vert"), GL_VERTEX_SHADER);
	fragment_shader = LoadShader(std::string("./shaders/fragment.frag"), GL_FRAGMENT_SHADER);

	shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);

	glBindFragDataLocation(shader_program, 0, "color_out");
	glAttachShader(shader_program, fragment_shader);

	glLinkProgram(shader_program);
	CheckLinkStatus(shader_program);

	glDetachShader(shader_program, vertex_shader);
	glDetachShader(shader_program, fragment_shader);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	glUseProgram(shader_program);

	mvp_uniform_attribute = glGetUniformLocation(shader_program, "mvp");
	model_matrix_uniform_attribute = glGetUniformLocation(shader_program, "model_matrix"); // Add this line
	light_position_uniform_attribute = glGetUniformLocation(shader_program, "light_position");
	camera_position_uniform_attribute = glGetUniformLocation(shader_program, "camera_position");
	texture_uniform_attribute = glGetUniformLocation(shader_program, "texture_sampler");
	use_texture_uniform_attribute = glGetUniformLocation(shader_program, "use_texture");
	material_shininess_uniform_attribute = glGetUniformLocation(shader_program, "material_shininess");
	material_specular_uniform_attribute = glGetUniformLocation(shader_program, "material_specular");
    ambient_value_uniform_attribute = glGetUniformLocation(shader_program, "ambient_value"); // Get location for ambient_value

	scene_loader = new SceneLoader(*this);

	// Load texture if model has one
	if (model.hasTexture()) {
		auto text = model.material.loadTexture();
		if (!text)
		{
			std::cerr << "failed to load texture";
		}
	}

	setupLightUniformLocations();
}

void View::render() {
    camera.updateViewMatrix();
    glm::mat4 model_view_projection = camera.projection_matrix * camera.view_matrix * model.matrix;
    
    // Update uniforms for transformation matrices
    glUniformMatrix4fv(mvp_uniform_attribute, 1, GL_FALSE, &model_view_projection[0][0]);
    glUniformMatrix4fv(model_matrix_uniform_attribute, 1, GL_FALSE, &model.matrix[0][0]);
    
    // Set the legacy light position for backward compatibility
    //glUniform3fv(light_position_uniform_attribute, 1, &light_position[0]);
    glUniform3fv(camera_position_uniform_attribute, 1, &camera.position[0]);

    // Set material properties uniforms
    glUniform1f(material_shininess_uniform_attribute, model.material.shininess);
    glUniform3fv(material_specular_uniform_attribute, 1, &model.material.specular_color[0]);
    glUniform1f(ambient_value_uniform_attribute, model.ambient_value); // Set ambient_value uniform

    // Set texture uniforms
    bool has_texture = model.hasTexture() && model.material.diffuse_texture_id != 0;
    glUniform1i(use_texture_uniform_attribute, has_texture ? 1 : 0);

    if (has_texture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, model.material.diffuse_texture_id);
        glUniform1i(texture_uniform_attribute, 0);  // Use texture unit 0
    }

    // Set light count uniforms
    glUniform1i(directional_light_count_uniform, static_cast<int>(directional_lights.size()));
    glUniform1i(point_light_count_uniform, static_cast<int>(point_lights.size()));
    glUniform1i(spotlight_count_uniform, static_cast<int>(spotlights.size()));

    // Set directional light uniforms
    for (size_t i = 0; i < directional_lights.size(); i++) {
        size_t base_idx = i * 3; // Each directional light has 3 uniforms
        glUniform3fv(directional_light_uniforms[base_idx], 1, &directional_lights[i].direction[0]);
        glUniform3fv(directional_light_uniforms[base_idx + 1], 1, &directional_lights[i].color[0]);
        glUniform1f(directional_light_uniforms[base_idx + 2], directional_lights[i].intensity);
    }

    // Set point light uniforms
    for (size_t i = 0; i < point_lights.size(); i++) {
        size_t base_idx = i * 6; // Each point light has 6 uniforms
        glUniform3fv(point_light_uniforms[base_idx], 1, &point_lights[i].position[0]);
        glUniform3fv(point_light_uniforms[base_idx + 1], 1, &point_lights[i].color[0]);
        glUniform1f(point_light_uniforms[base_idx + 2], point_lights[i].intensity);
        glUniform1f(point_light_uniforms[base_idx + 3], point_lights[i].constant);
        glUniform1f(point_light_uniforms[base_idx + 4], point_lights[i].linear);
        glUniform1f(point_light_uniforms[base_idx + 5], point_lights[i].quadratic);
    }

    // Set spotlight uniforms
    for (size_t i = 0; i < spotlights.size(); i++) {
        size_t base_idx = i * 9; // Each spotlight has 9 uniforms
        glUniform3fv(spotlight_uniforms[base_idx], 1, &spotlights[i].position[0]);
        glUniform3fv(spotlight_uniforms[base_idx + 1], 1, &spotlights[i].direction[0]);
        glUniform3fv(spotlight_uniforms[base_idx + 2], 1, &spotlights[i].color[0]);
        glUniform1f(spotlight_uniforms[base_idx + 3], spotlights[i].intensity);
        glUniform1f(spotlight_uniforms[base_idx + 4], spotlights[i].cutoff);
        glUniform1f(spotlight_uniforms[base_idx + 5], spotlights[i].outer_cutoff);
        glUniform1f(spotlight_uniforms[base_idx + 6], spotlights[i].constant);
        glUniform1f(spotlight_uniforms[base_idx + 7], spotlights[i].linear);
        glUniform1f(spotlight_uniforms[base_idx + 8], spotlights[i].quadratic);
    }

    // Bind the VAO before drawing
    glBindVertexArray(scene_loader->vertex_array_object);

    // Draw using vertex indices with glDrawElements
    glDrawElements(GL_TRIANGLES, model.indices.size(), GL_UNSIGNED_INT, 0);
}

View::~View()
{
	delete scene_loader;
}

void View::SceneLoader::loadModel(const Model& model) {
	// For each vertex in the model
	for (size_t i = 0; i < model.vertices.size(); i++) {
		// Position (xyz)
		model_data_vector.push_back(model.vertices[i].x);
		model_data_vector.push_back(model.vertices[i].y);
		model_data_vector.push_back(model.vertices[i].z);

		// Color (rgb)
		model_data_vector.push_back(model.color.r);
		model_data_vector.push_back(model.color.g);
		model_data_vector.push_back(model.color.b);

		// Normal (xyz)
		if (i < model.normals.size()) {
			model_data_vector.push_back(model.normals[i].x);
			model_data_vector.push_back(model.normals[i].y);
			model_data_vector.push_back(model.normals[i].z);
		}
		else {
			// Default normal if not provided
			model_data_vector.push_back(0.0f);
			model_data_vector.push_back(1.0f);
			model_data_vector.push_back(0.0f);
		}

		// Texture coordinates (uv)
		if (i < model.texture_coords.size()) {
			model_data_vector.push_back(model.texture_coords[i].x);
			model_data_vector.push_back(model.texture_coords[i].y);
		}
		else {
			// Default texture coordinates if not provided
			model_data_vector.push_back(0.0f);
			model_data_vector.push_back(0.0f);
		}
	}
}

void View::SceneLoader::setupBuffers() {
	// Generate a vertex array object
	glGenVertexArrays(1, &vertex_array_object);
	glBindVertexArray(vertex_array_object);

	// Create and fill the vertex buffer
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(float) * model_data_vector.size(),
		model_data_vector.data(),
		GL_STATIC_DRAW);

	// Create and fill the element buffer object (EBO)
	glGenBuffers(1, &element_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		sizeof(unsigned int) * view.model.indices.size(),
		view.model.indices.data(),
		GL_STATIC_DRAW);

	// Size of each vertex attribute:
	// 3 floats for position + 3 floats for color + 3 floats for normal + 2 floats for texture = 11 floats
	const int stride = 11 * sizeof(float);

	// Set up vertex attribute pointers
	GLint vertex_position_attribute = glGetAttribLocation(view.shader_program, "vertex_position");
	glVertexAttribPointer(vertex_position_attribute, 3, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(vertex_position_attribute);

	GLint color_input_attribute = glGetAttribLocation(view.shader_program, "color_input");
	glVertexAttribPointer(color_input_attribute, 3, GL_FLOAT, GL_FALSE, stride,
		(void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(color_input_attribute);

	GLint vertex_normal_attribute = glGetAttribLocation(view.shader_program, "vertex_normal_in");
	glVertexAttribPointer(vertex_normal_attribute, 3, GL_FLOAT, GL_FALSE, stride,
		(void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(vertex_normal_attribute);

	// Make sure texture coordinates are correctly bound
	GLint texture_coord_attribute = glGetAttribLocation(view.shader_program, "texture_coord_in");
	if (texture_coord_attribute != -1) { // Only set up if the attribute exists in the shader
		glVertexAttribPointer(texture_coord_attribute, 2, GL_FLOAT, GL_FALSE, stride,
			(void*)(9 * sizeof(float)));
		glEnableVertexAttribArray(texture_coord_attribute);
	}
}

View::SceneLoader::SceneLoader(View& view) :view(view) {
	loadModel(view.model);
	setupBuffers();
}

// Add this method implementation to set up light uniform locations
void View::setupLightUniformLocations() {
    // Get uniform locations for light counts
    directional_light_count_uniform = glGetUniformLocation(shader_program, "directional_light_count");
    point_light_count_uniform = glGetUniformLocation(shader_program, "point_light_count");
    spotlight_count_uniform = glGetUniformLocation(shader_program, "spotlight_count");
    
    // Get uniform locations for directional lights
    for (int i = 0; i < MAX_DIRECTIONAL_LIGHTS; ++i) {
        std::string base = "directional_lights[" + std::to_string(i) + "].";
        GLuint direction = glGetUniformLocation(shader_program, (base + "direction").c_str());
        GLuint color = glGetUniformLocation(shader_program, (base + "color").c_str());
        GLuint intensity = glGetUniformLocation(shader_program, (base + "intensity").c_str());
        
        directional_light_uniforms.push_back(direction);
        directional_light_uniforms.push_back(color);
        directional_light_uniforms.push_back(intensity);
    }
    
    // Get uniform locations for point lights
    for (int i = 0; i < MAX_POINT_LIGHTS; ++i) {
        std::string base = "point_lights[" + std::to_string(i) + "].";
        GLuint position = glGetUniformLocation(shader_program, (base + "position").c_str());
        GLuint color = glGetUniformLocation(shader_program, (base + "color").c_str());
        GLuint intensity = glGetUniformLocation(shader_program, (base + "intensity").c_str());
        GLuint constant = glGetUniformLocation(shader_program, (base + "constant").c_str());
        GLuint linear = glGetUniformLocation(shader_program, (base + "linear").c_str());
        GLuint quadratic = glGetUniformLocation(shader_program, (base + "quadratic").c_str());
        
        point_light_uniforms.push_back(position);
        point_light_uniforms.push_back(color);
        point_light_uniforms.push_back(intensity);
        point_light_uniforms.push_back(constant);
        point_light_uniforms.push_back(linear);
        point_light_uniforms.push_back(quadratic);
    }
    
    // Get uniform locations for spotlights
    for (int i = 0; i < MAX_SPOTLIGHTS; ++i) {
        std::string base = "spotlights[" + std::to_string(i) + "].";
        GLuint position = glGetUniformLocation(shader_program, (base + "position").c_str());
        GLuint direction = glGetUniformLocation(shader_program, (base + "direction").c_str());
        GLuint color = glGetUniformLocation(shader_program, (base + "color").c_str());
        GLuint intensity = glGetUniformLocation(shader_program, (base + "intensity").c_str());
        GLuint cutoff = glGetUniformLocation(shader_program, (base + "cutoff").c_str());
        GLuint outer_cutoff = glGetUniformLocation(shader_program, (base + "outer_cutoff").c_str());
        GLuint constant = glGetUniformLocation(shader_program, (base + "constant").c_str());
        GLuint linear = glGetUniformLocation(shader_program, (base + "linear").c_str());
        GLuint quadratic = glGetUniformLocation(shader_program, (base + "quadratic").c_str());
        
        spotlight_uniforms.push_back(position);
        spotlight_uniforms.push_back(direction);
        spotlight_uniforms.push_back(color);
        spotlight_uniforms.push_back(intensity);
        spotlight_uniforms.push_back(cutoff);
        spotlight_uniforms.push_back(outer_cutoff);
        spotlight_uniforms.push_back(constant);
        spotlight_uniforms.push_back(linear);
        spotlight_uniforms.push_back(quadratic);
    }
}

// Add the directional light method implementation
void View::addDirectionalLight(const glm::vec3& direction, const glm::vec3& color, float intensity) {
    // Only add if we haven't reached the maximum number of directional lights
    if (directional_lights.size() < MAX_DIRECTIONAL_LIGHTS) {
        DirectionalLight light;
        light.direction = glm::normalize(direction); // Normalize direction vector
        light.color = color;
        light.intensity = intensity;
        
        directional_lights.push_back(light);
    } else {
        std::cerr << "Maximum number of directional lights reached. Light not added." << std::endl;
    }
}

// Add the point light method implementation
void View::addPointLight(const glm::vec3& position, const glm::vec3& color, float intensity,
                        float constant, float linear, float quadratic) {
    // Only add if we haven't reached the maximum number of point lights
    if (point_lights.size() < MAX_POINT_LIGHTS) {
        PointLight light;
        light.position = position;
        light.color = color;
        light.intensity = intensity;
        light.constant = constant;
        light.linear = linear;
        light.quadratic = quadratic;
        
        point_lights.push_back(light);
    } else {
        std::cerr << "Maximum number of point lights reached. Light not added." << std::endl;
    }
}

// Add the spotlight method implementation
void View::addSpotLight(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& color,
                      float intensity, float cutoff, float outer_cutoff, float constant,
                      float linear, float quadratic) {
    // Only add if we haven't reached the maximum number of spotlights
    if (spotlights.size() < MAX_SPOTLIGHTS) {
        SpotLight light;
        light.position = position;
        light.direction = glm::normalize(direction); // Normalize direction vector
        light.color = color;
        light.intensity = intensity;
        light.cutoff = cutoff;
        light.outer_cutoff = outer_cutoff;
        light.constant = constant;
        light.linear = linear;
        light.quadratic = quadratic;
        
        spotlights.push_back(light);
    } else {
        std::cerr << "Maximum number of spotlights reached. Light not added." << std::endl;
    }
}

