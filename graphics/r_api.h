//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "r_vulkan_api.h"
#include "r_opengl_api.h"

#include "r_windowdata.h"
#include "r_renderdata.h"
#include "r_drawdata.h"
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

    DrawBufferID registerDrawBuffer(const GraphicsAPI &api, const std::vector<VertexData> &vertices, const std::vector<ui16> &indices);
    TextureID registerTexture(const GraphicsAPI &api, Vec2<> size, Channels ch, ui8* pixels);
    DrawID registerDrawData(GraphicsAPI &api, DrawBufferID buffer);
    inline std::map<DrawBufferID, DrawBufferData> draw_buffer_data{};
    inline std::map<TextureID, TextureData> texture_data{};
    inline std::map<DrawID, DrawData> draw_data{};
    inline std::map<const TextureData*, std::vector<DrawQueueInfo>> draw_queue_textures{};

    void updateDescriptorSets(const GraphicsAPI &api, const DrawData* draw);

    void resize(GraphicsAPI &api, WindowData &win);

    void renderTest(GraphicsAPI &api, WindowData &win, RenderData &render);

    void clean(GraphicsAPI &api);
}
