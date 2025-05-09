#pragma once

#include <string>
#include <vector>
#include <GL/glew.h>
#include <SOIL2/SOIL2.h>

// Class for loading texture images using SOIL
class TextureLoader {
public:
    // Load a texture from a file path and return the OpenGL texture ID
    static GLuint loadTexture(const std::string& path);
    
    // Free a texture by its ID
    static void freeTexture(GLuint textureID);
};