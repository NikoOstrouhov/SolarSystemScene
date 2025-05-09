#pragma once

#include <SDL2/SDL.h>
#include <iostream>
#include <gl/glew.h>


/*
Call to create a window, initialize a GL context and initialize the GLEW library
Necessary to start drawing to a window
*/
void init(SDL_Window*& window);

/*
Call before creating a GL context to set attributes
*/
void initGLAttributes(int glMajorVersion, int glMinorVersion, int useDoubleBuffering);

/*
Call when program is done
*/
void tearDown(SDL_Window*& window);
