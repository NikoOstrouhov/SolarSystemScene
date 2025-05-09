#include "material.h"
#include "texture_loader.h"

Material::Material(const glm::vec3& diffuse, const glm::vec3& ambient, 
                  const glm::vec3& specular, float shininess) :
    diffuse_color(diffuse),
    ambient_color(ambient),
    specular_color(specular),
    shininess(shininess),
    diffuse_texture_id(0)
{
}

bool Material::hasTexture() const {
    return !diffuse_texture_path.empty();
}

bool Material::loadTexture() {
    if (!hasTexture()) {
        return false;
    }
    
    // Free existing texture if any
    freeTexture();
    
    // Load the texture
    diffuse_texture_id = TextureLoader::loadTexture(diffuse_texture_path);
    return (diffuse_texture_id != 0);
}

void Material::freeTexture() {
    if (diffuse_texture_id != 0) {
        TextureLoader::freeTexture(diffuse_texture_id);
        diffuse_texture_id = 0;
    }
}