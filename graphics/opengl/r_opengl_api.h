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
    
    //Render passes
    //----------------------------------------
    AttachmentID registerAttachment(std::map<AttachmentID, AttachmentData> &attachments, Vec2<> size, AttachmentType type);
    ui32 createAttachmentTexture(Vec2<> size, AttachmentType type);
    SubpassID registerSubpass(std::map<SubpassID, SubpassData> &subpasses, const std::map<AttachmentID, AttachmentData> &attachments,
                              std::vector<AttachmentID> list);
    ui32 createFramebuffer(const std::map<AttachmentID, AttachmentData> &attachments, std::vector<AttachmentID> list);
    //----------------------------------------

    //Shaders
    //----------------------------------------
    ShaderData createShaderDataGL(str name, SubpassID subpass);
    ui8 compileShader(const char* source, ui32 shader_type);
    ui8 compileProgram(str vert_source = "", str frag_source = "");
    void validateShaderData(ui32 vao_id, const std::map<Shaders, ShaderData> &shaders);
    //----------------------------------------

    //Vertices
    //----------------------------------------
    ui32 createVertexArray();
    //----------------------------------------
    
    //Buffers
    //----------------------------------------
    BufferData createBuffer(size_t size = 0, GLenum type = GL_UNIFORM_BUFFER, GLenum usage = GL_STATIC_DRAW);

    template <typename T>
    void updateUniformBuffer(BufferData buffer, const T* uniform) {
        glBindBuffer(GL_UNIFORM_BUFFER, buffer.id_);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(T), uniform);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    };

    BufferData createIndexBuffer(const GraphicsAPI &api, const std::vector<ui16> &indices);
    
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

    //GUI
    //----------------------------------------
    namespace GUI
    {
        void initImGUI(OpenGL &gl, const WindowData &win);
    }
    //----------------------------------------
}

namespace Fresa::Graphics::API
{
    template <typename V, std::enable_if_t<Reflection::is_reflectable<V>, bool> = true>
    DrawBufferID registerDrawBuffer(const GraphicsAPI &api, const std::vector<V> &vertices, const std::vector<ui16> &indices) {
        static DrawBufferID id = 0;
        do id++;
        while (draw_buffer_data.find(id) != draw_buffer_data.end());
        
        draw_buffer_data[id] = DrawBufferData{};
        
        auto [vb_, vao_] = GL::createVertexBuffer(api, vertices);
        draw_buffer_data[id].vertex_buffer = vb_;
        draw_buffer_data[id].vao = vao_;
        draw_buffer_data[id].index_buffer = GL::createIndexBuffer(api, indices);
        draw_buffer_data[id].index_size = (ui32)indices.size();
        
        return id;
    }
}

#endif
