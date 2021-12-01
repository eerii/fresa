//project fresa, 2017-2022
//by jose pazos perez
//all rights reserved uwu

#pragma once

#ifdef USE_OPENGL

#include "r_api.h"

namespace Fresa::Graphics::GL
{
    //Initialization
    //----------------------------------------
    SDL_GLContext createContext(const WindowData &win);
    //----------------------------------------

    //Shaders
    //----------------------------------------
    ShaderData createShaderDataGL(str name);
    ui8 compileShader(const char* source, ui32 shader_type);
    ui8 compileProgram(str vert_source = "", str frag_source = "");
    void validateShaderData(ui32 vao_id, const std::map<str, ShaderData> &shaders);
    //----------------------------------------

    //Vertices
    //----------------------------------------
    ui32 createVertexArray();
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

    std::pair<BufferData, ui32> createVertexBuffer(const GraphicsAPI &api, const std::vector<Graphics::VertexData> &vertices);
    BufferData createIndexBuffer(const GraphicsAPI &api, const std::vector<ui16> &indices);
    
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
