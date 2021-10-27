//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#ifdef USE_OPENGL

#include "r_opengl.h"

#include "r_windowdata.h"
#include "r_renderdata.h"

namespace Verse::Graphics::GL
{
    void config();
    OpenGL create(WindowData &win);

    namespace Init
    {
        void createContext(OpenGL &gl, WindowData &win);
        void createShaderData(OpenGL &gl);
    
        void createFramebuffers(OpenGL &gl, WindowData &win);
        void createVertexArrays(OpenGL &gl);
        void validateShaderData(OpenGL &gl);
        void createVertexBuffers(OpenGL &gl);
        void configureVertexAttributes(OpenGL &gl);
        void configureProperties();
    
        void initImGUI(OpenGL &gl, WindowData &win);
    }

    namespace Buffers
    {
        FramebufferData createFramebuffer(Vec2<> size, FramebufferType type);
        VertexArrayData createVertexArray();
        BufferData createVertexBuffer(VertexArrayData &vao);
    }

    void renderTest(WindowData &win, RenderData &render);
}

#endif
