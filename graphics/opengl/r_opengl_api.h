//: fresa by jose pazos perez, licensed under GPLv3
#pragma once

#ifdef USE_OPENGL

#include "r_api.h"

namespace Fresa::Graphics::GL
{
    //Initialization
    //----------------------------------------
    SDL_GLContext createContext();
    //----------------------------------------
    
    //Render passes
    //----------------------------------------
    ui32 createAttachmentTexture(Vec2<ui16> size, AttachmentType type);
    ui32 createFramebuffer(SubpassID subpass);
    //----------------------------------------

    //Shaders
    //----------------------------------------
    ShaderData createShaderDataGL(ShaderID shader);
    ui8 compileProgram(const std::vector<ui32> &vert_source, const std::vector<ui32> &frag_source);
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
    std::pair<BufferData, ui32> createVertexBuffer(const GraphicsAPI &api, const std::vector<V> &vertices,
                                                   std::vector<VertexAttributeDescription> attributes = {},
                                                   ui32 vao_ = -1) {
        //---Vertex buffer---
        //      It holds the vertices for the vertex shader to read
        //      It needs to be tied to the vertex array object (vao)
        ui32 vao = (vao_ == -1) ? GL::createVertexArray() : vao_;
        glBindVertexArray(vao);
        
        BufferData buffer = GL::createBuffer();
        glBindBuffer(GL_ARRAY_BUFFER, buffer.id_);
        
        if (attributes.size() == 0)
            attributes = getAttributeDescriptions<V>();
        
        for (const auto &attr : attributes) {
            ui32 size = (ui32)attr.format; //Assuming float
            glVertexAttribPointer(attr.location, size, GL_FLOAT, GL_FALSE, sizeof(V), reinterpret_cast<void*>(attr.offset));
            if (attr.binding == 1) //: Instancing buffer, update each instance, not each vertex
                glVertexAttribDivisor(attr.location, 1);
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

namespace Fresa::Graphics
{
    struct GlDescriptorPoolSize {
        ShaderDescriptorType type;
        ui32 descriptorCount;
    };
    
    struct GlDescriptorPool {
        std::vector<GlDescriptorPoolSize> sizes;
        //: ...
    };
    
    struct GlDescriptorSet {
        //: ...
    };
    
    template <typename... UBO>
    DrawUniformID registerDrawUniforms(ShaderID shader) {
        static DrawUniformID id = 0;
        do id++;
        while (api.draw_uniform_data.find(id) != api.draw_uniform_data.end());
        
        api.draw_uniform_data[id] = DrawUniformData{};
        DrawUniformData &data = api.draw_uniform_data.at(id);
        
        ([&](){
            data.uniform_buffers.push_back(GL::createBuffer(sizeof(UBO), GL_UNIFORM_BUFFER, GL_STREAM_DRAW));
        }(), ...);
        
        return id;
    }
    
    template <typename UBO>
    void updateUniformBuffer(BufferData buffer, const UBO& ubo) {
        glBindBuffer(GL_UNIFORM_BUFFER, buffer.id_);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UBO), &ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
    
    template <typename... UBO>
    void updateDrawUniformBuffer(DrawDescription &description, const UBO& ...ubo) {
        DrawUniformData &uniform = api.draw_uniform_data.at(description.uniform);
        int i = 0;
        ([&](){
            updateUniformBuffer(uniform.uniform_buffers.at(i++), ubo);
        }(), ...);
    }
    
    template <typename... UBO>
    void updateComputeUniformBuffers(ShaderID shader, const UBO& ...ubo) { }
    
    template <typename V, typename I, std::enable_if_t<Reflection::is_reflectable<V> && std::is_integral_v<I>, bool> = true>
    GeometryBufferID registerGeometryBuffer(const std::vector<V> &vertices, const std::vector<I> &indices) {
        static GeometryBufferID id = 0;
        do id++;
        while (api.geometry_buffer_data.find(id) != api.geometry_buffer_data.end());
        
        api.geometry_buffer_data[id] = GeometryBufferData{};
        GeometryBufferData &data = api.geometry_buffer_data.at(id);
        
        auto [vb, vao] = GL::createVertexBuffer(api, vertices);
        data.vertex_buffer = vb;
        data.vao = vao;
        
        data.index_buffer = GL::createIndexBuffer(api, indices);
        data.index_size = (ui32)indices.size();
        data.index_bytes = (ui8)sizeof(I);
        
        return id;
    }
    
    template <typename V, typename U, std::enable_if_t<Reflection::is_reflectable<V> && Reflection::is_reflectable<U>, bool> = true>
    InstancedBufferID registerInstancedBuffer(const std::vector<V> &vertices, const std::vector<U> &instanced_data, ui32 vao) {
        static InstancedBufferID id = 0;
        do id++;
        while (api.instanced_buffer_data.find(id) != api.instanced_buffer_data.end());
        
        api.instanced_buffer_data[id] = InstancedBufferData{};
        InstancedBufferData &data = api.instanced_buffer_data.at(id);
        
        //: Get only the instanced attributes with updated positions
        auto attributes = getAttributeDescriptions<V, U>();
        attributes.erase(attributes.begin(), attributes.begin() + getAttributeDescriptions<V>().size());
        auto [inst_vb, _] = GL::createVertexBuffer(api, instanced_data, attributes, vao);
        data.instance_buffer = inst_vb;
        data.instance_count = (ui32)instanced_data.size();
        
        return id;
    }
    
    template <typename V>
    void updateBufferFromCompute(const BufferData &buffer, ui32 buffer_size, ShaderID shader, std::function<std::vector<V>()> fallback) {
        static_assert(sizeof(V) % sizeof(glm::vec4) == 0, "The buffer should be aligned to a vec4 (4 floats) for the compute shader padding to match");
        #ifdef HAS_COMPUTE
            static_assert(false, "The OpenGL renderer should not have compute capabilities right now, check if you are setting the correct macro");
        #else
            const std::vector<V> data = fallback();
            glBindBuffer(GL_ARRAY_BUFFER, buffer.id_);
            glBufferSubData(GL_ARRAY_BUFFER, 0, buffer_size * sizeof(V), data.data());
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        #endif
    }
}

#endif
