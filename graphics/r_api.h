//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "r_vulkan_api.h"
#include "r_opengl_api.h"

#include "r_windowdata.h"
#include "r_renderdata.h"
#include "r_texturedata.h"
#include "r_bufferdata.h"

#include "log.h"

namespace Verse::Graphics
{
#if defined USE_VULKAN
    using GraphicsAPI = Vulkan;
#elif defined USE_OPENGL
    using GraphicsAPI = OpenGL;
#endif
}

namespace Verse::Graphics::API
{
    void configure();
    GraphicsAPI create(WindowData &win);

    BufferData createVertexBuffer(const GraphicsAPI &api, const std::vector<Graphics::VertexData> &vertices);
    BufferData createIndexBuffer(const GraphicsAPI &api, const std::vector<ui16> &indices);

    DrawBufferID registerDrawBuffer(GraphicsAPI &api, const std::vector<VertexData> &vertices, const std::vector<ui16> &indices);
    DrawID registerDrawData(GraphicsAPI &api, DrawBufferID buffer);
    inline std::map<DrawBufferID, DrawBuffer> draw_buffers{};
    inline std::map<DrawID, DrawData> draw_data{};

    void createTexture(GraphicsAPI &api, TextureData &tex, ui8* pixels);
    void resize(GraphicsAPI &api, WindowData &win);

    inline std::map<const TextureData*, std::vector<DrawQueueInfo>> draw_queue{};
    inline void draw(const TextureData &tex, const DrawID draw_id, glm::mat4 model) {
        DrawQueueInfo queue_info{};
        
        queue_info.data = &draw_data.at(draw_id);
        if (queue_info.data == nullptr)
            log::error("Tried to draw an object with a null pointer draw data");
        
        queue_info.buffer = &draw_buffers.at(queue_info.data->buffer_id);
        if (queue_info.buffer == nullptr)
            log::error("Tried to draw an object with a null pointer draw buffer");
        
        queue_info.model = model;
        
        draw_queue[&tex].push_back(queue_info);
    }

    void renderTest(GraphicsAPI &api, WindowData &win, RenderData &render);

    void clean(GraphicsAPI &api);
}
