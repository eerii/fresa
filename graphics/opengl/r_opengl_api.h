//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#ifdef USE_OPENGL

#include "r_api.h"

namespace Fresa::Graphics::GL
{
    //Initialization
    //----------------------------------------
    SDL_GLContext createContext(const WindowData &win);
    //----------------------------------------
    
    //Render passes
    //----------------------------------------
    ui32 createAttachmentTexture(Vec2<> size, AttachmentType type);
    ui32 createFramebuffer(SubpassID subpass);
    //----------------------------------------

    //Shaders
    //----------------------------------------
    ShaderData createShaderDataGL(ShaderID shader, SubpassID subpass);
    ui8 compileShader(const char* source, ui32 shader_type);
    ui8 compileProgram(str vert_source = "", str frag_source = "");
    void validateShaderData(ui32 vao_id);
    //----------------------------------------

    //Vertices
    //----------------------------------------
    ui32 createVertexArray();
    //----------------------------------------
    
    //Buffers
    //----------------------------------------
    BufferData createBuffer(size_t size = 0, GLenum type = GL_UNIFORM_BUFFER, GLenum usage = GL_STATIC_DRAW);
    
    template <typename I, std::enable_if_t<std::is_integral_v<I>, bool> = true>
    BufferData createIndexBuffer(const GraphicsAPI &api, const std::vector<I> &indices) {
        //---Index buffer---
        //      We are going to draw the mesh indexed, which means that vertex data is not repeated and we need a list of which vertices to draw
        BufferData buffer = GL::createBuffer();
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.id_);
        
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(I), indices.data(), GL_STATIC_DRAW);
        
        glCheckError();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        
        return buffer;
    }
    
    template <typename V, std::enable_if_t<Reflection::is_reflectable<V>, bool> = true>
    std::pair<BufferData, ui32> createVertexBuffer(const GraphicsAPI &api, const std::vector<V> &vertices) {
        //---Vertex buffer---
        //      It holds the vertices for the vertex shader to read
        //      It needs to be tied to the vertex array object (vao)
        ui32 vao = GL::createVertexArray();
        glBindVertexArray(vao);
        
        BufferData buffer = GL::createBuffer();
        glBindBuffer(GL_ARRAY_BUFFER, buffer.id_);
        
        static std::vector<VertexAttributeDescription> attributes = API::getAttributeDescriptions<V>();
        
        for (const auto &attr : attributes) {
            ui32 size = (ui32)attr.format; //Assuming float
            glVertexAttribPointer(attr.location, size, GL_FLOAT, GL_FALSE, sizeof(V), reinterpret_cast<void*>(attr.offset));
            glEnableVertexAttribArray(attr.location);
        }
        
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(V), vertices.data(), GL_STATIC_DRAW);
        
        glCheckError();
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        
        return {buffer, vao};
    }
    
    //----------------------------------------
}

namespace Fresa::Graphics::API
{
    template <typename UBO>
    DrawUniformID registerDrawUniforms(GraphicsAPI &api, ShaderID shader) {
        static DrawUniformID id = 0;
        do id++;
        while (draw_uniform_data.find(id) != draw_uniform_data.end());
        
        draw_uniform_data[id] = DrawUniformData{};
        DrawUniformData &data = draw_uniform_data.at(id);
        
        data.uniform_buffers = { GL::createBuffer(sizeof(UBO), GL_UNIFORM_BUFFER, GL_STREAM_DRAW) };
        
        return id;
    }
    
    template <typename V, typename I, std::enable_if_t<Reflection::is_reflectable<V> && std::is_integral_v<I>, bool> = true>
    GeometryBufferID registerGeometryBuffer(const GraphicsAPI &api, const std::vector<V> &vertices, const std::vector<I> &indices) {
        static GeometryBufferID id = 0;
        do id++;
        while (geometry_buffer_data.find(id) != geometry_buffer_data.end());
        
        geometry_buffer_data[id] = GeometryBufferData{};
        GeometryBufferData &data = geometry_buffer_data.at(id);
        
        auto [vb, vao] = GL::createVertexBuffer(api, vertices);
        data.vertex_buffer = vb;
        data.vao = vao;
        data.index_buffer = GL::createIndexBuffer(api, indices);
        data.index_size = (ui32)indices.size();
        data.index_bytes = (ui8)sizeof(I);
        
        return id;
    }
}

#endif
