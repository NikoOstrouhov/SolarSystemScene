#include "glinit.h"


/*
Call to create a window, initialize a GL context and initialize the GLEW library
Necessary to start drawing to a window
*/
void init(SDL_Window*& window) {
    SDL_Init(SDL_INIT_EVERYTHING);
    initGLAttributes(3, 3, 1);
    window = SDL_CreateWindow("cornell_box", 20, 40, 1488, 1488, SDL_WINDOW_OPENGL);
    if (window == NULL) {
        std::cout << "Failed to create window at startup";
    }
    SDL_GL_CreateContext(window);
    //swap buffer at the monitors rate
    SDL_GL_SetSwapInterval(1);

    //GLEW is an OpenGL Loading Library used to reach GL functions
    //Sets all functions available
    glewExperimental = GL_TRUE;
    GLenum glew_status = glewInit();
    if (glew_status != GLEW_OK) {
        std::cerr << "GLEW initialization failed: " << glewGetErrorString(glew_status) << std::endl;
    }
}

/*
Call before creating a GL context to set attributes
Used by init
*/
void initGLAttributes(int glMajorVersion, int glMinorVersion, int useDoubleBuffering) {
    //Disable deprecated functions
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    //Select version
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, glMajorVersion);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, glMinorVersion);
    //Use double buffer
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, useDoubleBuffering);
}

/*
Call when program is done
*/
void tearDown(SDL_Window*& window) {
    SDL_GL_DeleteContext(window);
    SDL_DestroyWindow(window);

    // Clean up SDL
    SDL_Quit();
}
