//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#ifdef USE_OPENGL

#include "r_opengl.h"

#include "r_windowdata.h"
#include "r_shader.h"

namespace Verse::Graphics::GL
{
    void config();
    OpenGL create(WindowData &win);

    namespace Init
    {
        void createContext(OpenGL &gl, WindowData &win);
        void createShaderData(OpenGL &gl);
        void initImGUI(OpenGL &gl, WindowData &win);
    }
}

#endif
