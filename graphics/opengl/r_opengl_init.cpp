//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#ifdef USE_OPENGL

#include "r_opengl_core.h"

using namespace Verse;
using namespace Graphics;

void GL::configOpenGL() {
    #ifdef __EMSCRIPTEN__
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    #else
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        #ifndef __APPLE__
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        #endif
    #endif
}

void GL::initOpenGL(OpenGL *gl, WindowData &win) {
    gl->createContext(win);
    
    #ifndef DISABLE_GUI
    gl->initImGUI(win);
    #endif
    
    gl->createShaderData();
}

#endif
