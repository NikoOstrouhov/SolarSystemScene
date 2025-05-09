#include "texture_loader.h"
#include <iostream>

GLuint TextureLoader::loadTexture(const std::string& path) {
    // Load image using SOIL
    GLuint textureID = SOIL_load_OGL_texture(
        path.c_str(),
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
    );
    
    // Check for errors
    if (textureID == 0) {
        std::cerr << "SOIL loading error: " << SOIL_last_result() << std::endl;
        return 0;
    }
    
    // Configure texture parameters
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    return textureID;
}

void TextureLoader::freeTexture(GLuint textureID) {
    if (textureID != 0) {
        glDeleteTextures(1, &textureID);
    }
}