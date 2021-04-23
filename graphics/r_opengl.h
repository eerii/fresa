//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "config.h"

#ifdef __APPLE__ //MACOS

    #define GL_SILENCE_DEPRECATION

    #include <OpenGL/OpenGL.h>
    #include <OpenGL/gl3.h>

#else //OTHERS (Not tested)

    #include <SDL2/SDL_opengl.h>
    #include <SDL2/SDL_opengl_glext.h>

#endif //!__APPLE__
