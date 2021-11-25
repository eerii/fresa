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

namespace Verse::Graphics::API
{
    void configure();
    GraphicsAPI create(WindowData &win);

    BufferData createVertexBuffer(const GraphicsAPI &api, const std::vector<Graphics::VertexData> &vertices);
    BufferData createIndexBuffer(const GraphicsAPI &api, const std::vector<ui16> &indices);
    DrawData createDrawData(const GraphicsAPI &api, const std::vector<VertexData> &vertices, const std::vector<ui16> &indices);
    DrawID registerDrawData(const GraphicsAPI &api, const std::vector<VertexData> &vertices, const std::vector<ui16> &indices);
    inline std::map<DrawID, DrawData> draw_data{};

    void createTexture(GraphicsAPI &api, TextureData &tex, ui8* pixels);
    void resize(GraphicsAPI &api, WindowData &win);

    void draw(const TextureData &tex, DrawID draw_id);
    inline std::map<const TextureData*, std::vector<const DrawData*>> draw_queue{};

    void renderTest(WindowData &win, RenderData &render, const DrawData* draw_data);

    void clean(GraphicsAPI &api);
}
