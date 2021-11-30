//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#ifdef USE_OPENGL

#include "r_opengl.h"

#include "r_windowdata.h"
#include "r_renderdata.h"
#include "r_shaderdata.h"
#include "r_vertexdata.h"

namespace Verse::Graphics::GL
{
    using VAO = ui32;

    //Initialization
    //----------------------------------------
    SDL_GLContext createContext(const WindowData &win);
    //----------------------------------------

    //Shaders
    //----------------------------------------
    ShaderData createShaderDataGL(str name);
    ui8 compileShader(const char* source, ui32 shader_type);
    ui8 compileProgram(str vert_source = "", str frag_source = "");
    void validateShaderData(VAO vao_id, const std::map<str, ShaderData> &shaders);
    //----------------------------------------

    //Vertices
    //----------------------------------------
    template <typename V, std::enable_if_t<Reflection::is_reflectable<V>, bool> = true>
    VertexArrayData createVertexArray() {
        //---Vertex array---
        //      Stores the layout of per vertex data that will be passed to the shader later
        VertexArrayData vao;
        glGenVertexArrays(1, &vao.id_);
        vao.attributes = API::getAttributeDescriptions<V>();
        glCheckError();
        return vao;
    }
    //----------------------------------------

    //Buffers
    //----------------------------------------
    FramebufferData createFramebuffer(Vec2<> size, FramebufferType type);

    BufferData createBuffer(size_t size = 0, GLenum type = GL_UNIFORM_BUFFER, GLenum usage = GL_STATIC_DRAW);

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
        void initImGUI(OpenGL &gl, const WindowData &win);
    }
    //----------------------------------------
}

#endif
