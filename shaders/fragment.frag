#version 330 core
 
in vec3 fragment_color;
in vec3 fragment_position;
in vec3 vertex_normal;
uniform float ambient_value;
in vec2 texture_coord;

// Light structure definitions
#define MAX_DIRECTIONAL_LIGHTS 4
#define MAX_POINT_LIGHTS 4
#define MAX_SPOTLIGHTS 4

struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    float cutoff;
    float outer_cutoff;
    float constant;
    float linear;
    float quadratic;
};

// Light uniform arrays
uniform DirectionalLight directional_lights[MAX_DIRECTIONAL_LIGHTS];
uniform int directional_light_count;

uniform PointLight point_lights[MAX_POINT_LIGHTS];
uniform int point_light_count;

uniform SpotLight spotlights[MAX_SPOTLIGHTS];
uniform int spotlight_count;

// Keeping the original light for backward compatibility
//uniform vec3 light_position;

// Other uniforms
uniform vec3 camera_position;
uniform sampler2D texture_sampler;
uniform int use_texture;
uniform float material_shininess;
uniform vec3 material_specular;
 
out vec3 color_out;

// Function to calculate directional light contribution
vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 view_dir, vec3 material_color) {
    vec3 light_dir = normalize(-light.direction);
    
    // Diffuse component
    float diff = max(dot(normal, light_dir), 0.0);
    vec3 diffuse = light.color * diff * material_color * light.intensity;
    
    // Specular component (Blinn-Phong)
    vec3 halfway_dir = normalize(light_dir + view_dir);
    float spec = pow(max(dot(normal, halfway_dir), 0.0), material_shininess);
    vec3 specular = light.color * spec * material_specular * light.intensity;
    
    return diffuse + specular;
}

// Function to calculate point light contribution
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 frag_pos, vec3 view_dir, vec3 material_color) {
    vec3 light_dir = normalize(light.position - frag_pos);
    
    // Diffuse component
    float diff = max(dot(normal, light_dir), 0.0);
    vec3 diffuse = light.color * diff * material_color * light.intensity;
    
    // Specular component (Blinn-Phong)
    vec3 halfway_dir = normalize(light_dir + view_dir);
    float spec = pow(max(dot(normal, halfway_dir), 0.0), material_shininess);
    vec3 specular = light.color * spec * material_specular * light.intensity;
    
    // Attenuation
    float distance = length(light.position - frag_pos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
    
    diffuse *= attenuation;
    specular *= attenuation;
    
    return diffuse + specular;
}

// Function to calculate spotlight contribution
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 frag_pos, vec3 view_dir, vec3 material_color) {
    vec3 light_dir = normalize(light.position - frag_pos);
    
    // Diffuse component
    float diff = max(dot(normal, light_dir), 0.0);
    vec3 diffuse = light.color * diff * material_color * light.intensity;
    
    // Specular component (Blinn-Phong)
    vec3 halfway_dir = normalize(light_dir + view_dir);
    float spec = pow(max(dot(normal, halfway_dir), 0.0), material_shininess);
    vec3 specular = light.color * spec * material_specular * light.intensity;
    
    // Attenuation
    float distance = length(light.position - frag_pos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
    
    // Spotlight intensity (soft edges)
    float theta = dot(light_dir, normalize(-light.direction));
    float epsilon = light.cutoff - light.outer_cutoff;
    float intensity = clamp((theta - light.outer_cutoff) / epsilon, 0.0, 1.0);
    
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    
    return diffuse + specular;
}
 
void main(){
    vec3 material_color;
    
    // Use texture if available, otherwise use fragment color
    if (use_texture == 1) {
        material_color = texture(texture_sampler, texture_coord).rgb;
    } else {
        material_color = fragment_color;
    }
    
    // Ambient light component (constant)
   vec3 ambient = ambient_value* material_color;
 
    // Normalize the normal vector
    vec3 normal = normalize(vertex_normal);
    vec3 view_dir = normalize(camera_position - fragment_position);
    
    // Initialize lighting result with ambient
    vec3 result = ambient;
    
          // Calculate directional lights contributions
        for (int i = 0; i < directional_light_count; i++) {
            result += CalcDirectionalLight(directional_lights[i], normal, view_dir, material_color);
        }
        
        // Calculate point lights contributions
        for (int i = 0; i < point_light_count; i++) {
            result += CalcPointLight(point_lights[i], normal, fragment_position, view_dir, material_color);
        }
        
        // Calculate spotlights contributions
        for (int i = 0; i < spotlight_count; i++) {
            result += CalcSpotLight(spotlights[i], normal, fragment_position, view_dir, material_color);
        }
    
    // Apply basic gamma correction for better color representation
    result = pow(result, vec3(1.0/2.2));
    
    color_out = result;
}