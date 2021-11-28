//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#ifdef USE_OPENGL

#include "r_opengl.h"

#include "r_windowdata.h"
#include "r_renderdata.h"

#include "r_vertex.h"

namespace Verse::Graphics::GL
{
    //Initialization
    //----------------------------------------
    void createContext(OpenGL &gl, WindowData &win);
    void createShaderData(OpenGL &gl);

    void createFramebuffers(OpenGL &gl, WindowData &win);
    void createVertexArrays(OpenGL &gl);
    void validateShaderData(OpenGL &gl);
    void configureProperties();
    //----------------------------------------


    //Buffers
    //----------------------------------------
    FramebufferData createFramebuffer(Vec2<> size, FramebufferType type);

    template <typename V, std::enable_if_t<Reflection::is_reflectable<V>, bool> = true>
    VertexArrayData createVertexArray() {
        VertexArrayData vao;
        glGenVertexArrays(1, &vao.id_);
        vao.attributes = Vertex::getAttributeDescriptions<V>();
        glCheckError();
        return vao;
    }

    BufferData createBuffer(const VertexArrayData &vao, size_t size = 0, GLenum type = GL_UNIFORM_BUFFER, GLenum usage = GL_STATIC_DRAW);

    template <typename T>
    void updateUniformBuffer(BufferData buffer, const T* uniform) {
        glBindBuffer(GL_UNIFORM_BUFFER, buffer.id_);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(T), uniform);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    };
    //----------------------------------------


    //GUI
    //----------------------------------------
    namespace GUI
    {
        void initImGUI(OpenGL &gl, WindowData &win);
    }
    //----------------------------------------
}

#endif
