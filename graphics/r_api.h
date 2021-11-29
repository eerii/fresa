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
    inline std::map<const TextureData*, std::vector<DrawQueueInfo>> draw_queue{};

    void createTexture(GraphicsAPI &api, TextureData &tex, ui8* pixels);
    void resize(GraphicsAPI &api, WindowData &win);

    void renderTest(GraphicsAPI &api, WindowData &win, RenderData &render);

    void clean(GraphicsAPI &api);
}
