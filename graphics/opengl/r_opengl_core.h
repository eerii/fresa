//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#ifdef USE_OPENGL

#include "r_opengl.h"
#include "r_shader.h"
#include "gui.h"

namespace Verse::Graphics
{
    struct OpenGL;

    namespace GL
    {
        void configOpenGL();
        void configWebGL();
    
        void initOpenGL(OpenGL *gl, Config &c);
    }

    struct OpenGL {
        SDL_GLContext context;
        void createContext(Config &c);
        
        #ifndef DISABLE_GUI
        ImGuiContext* imgui_context;
        ImGuiIO io;
        void initImGUI(Config &c);
        #endif
        
        std::map<str, ShaderData> shaders;
        std::map<str, std::vector<str>> shader_locations;
        void createShaderData();
    };
}

#endif
