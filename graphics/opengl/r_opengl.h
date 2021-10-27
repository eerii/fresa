//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#ifdef USE_OPENGL

    #ifdef __EMSCRIPTEN__ //WEB

        #include <GLES3/gl3.h>

    #elif __APPLE__ //MACOS

        #define GL_SILENCE_DEPRECATION

        #include <OpenGL/OpenGL.h>
        #include <OpenGL/gl3.h>

    #else //OTHERS (Not tested)

        #include <SDL2/SDL_opengl.h>
        #include <SDL2/SDL_opengl_glext.h>

    #endif

#include "r_opengl_debug.h"

#include "r_shaderdata.h"
#include "r_bufferdata.h"
#include "r_framebufferdata.h"
#include "r_vertexdata.h"

#include "gui.h"

#include <map>

namespace Verse::Graphics
{
    struct OpenGL {
        SDL_GLContext context;
        
        std::map<str, ShaderData> shaders;
        std::map<str, std::vector<str>> shader_locations;
        
        FramebufferData framebuffer;
        VertexArrayData vao;
        BufferData vbo;
        
        #ifndef DISABLE_GUI
        ImGuiContext* imgui_context;
        ImGuiIO io;
        #endif
    };
}

#endif
