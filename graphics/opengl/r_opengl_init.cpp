//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#ifdef USE_OPENGL

#include "r_opengl_core.h"
#include "r_window.h"

using namespace Verse;
using namespace Graphics;

bool Verse::Graphics::GL::initOpenGL(OpenGL *gl, Config &c) {
#ifdef __EMSCRIPTEN__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    
    SDL_Renderer *renderer = NULL;
    SDL_CreateWindowAndRenderer(c.window_size.x, c.window_size.y, SDL_WINDOW_OPENGL, &c.window, &renderer);
#else
    //OPENGL
    #ifdef USE_OPENGL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        #ifndef __APPLE__
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        #endif
    #endif
    
    //CREATE A WINDOW
    c.window = Window::createWindow(c);
    if(c.window == nullptr) {
        log::error("There was an error with the window pointer, check r_pipeline");
        return false;
    }
#endif
    return true;
}

#endif
