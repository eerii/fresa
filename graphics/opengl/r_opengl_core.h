//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#ifdef USE_OPENGL

#include "r_opengl.h"

namespace Verse::Graphics
{
    struct OpenGL;

    namespace GL
    {
        bool initOpenGL(OpenGL *gl, Config &c);
    }
}

#endif
