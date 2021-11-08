//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "r_next_renderer.h"

#include "r_graphics.h"
#include "r_window.h"

using namespace Verse;
using namespace Graphics;

// What is needed for render?
// - Texture data (image id for gl or image views for vk)
//      Since binding images is expensive, it is best if all resources with the same image get rendered one after the other
// - Vertex data (can be a generic square, circle, triangle...)
// - Model matrix
// - Maybe the index of the shader to be used, but leave it for now
// - Batching and the rest can come later
// I need to store this in a structure that groups all the members with the same texture together, and can iterate between them
// Vertex data should be a pointer, texture data probably too, the matrix can be actual data

// What structure can I use?
// map<Texture*, Vertex+Model>, then use for(auto [a, b] : map)
// map<Texture*, std::pair<vertex+model>>

namespace {
    std::map<const TextureData*, std::vector<const DrawData*>> draw_queue;

    std::vector<VertexData> rect_vertices = {
        {{-1.f,  1.f,  0.f}, {0.f, 1.f, 0.f}},
        {{ 1.f,  1.f,  0.f}, {1.f, 1.f, 0.f}},
        {{-1.f, -1.f,  0.f}, {0.f, 0.f, 0.f}},
        {{ 1.f, -1.f,  0.f}, {1.f, 0.f, 0.f}},
    };
    std::vector<ui16> rect_indices = {
        0, 1, 2, 2, 3, 0
    };
}

RenderData Renderer::create(WindowData &win, Vec2<> resolution, bool vsync) {
    RenderData render;
    
    //Create the graphics API
    render.api = API::create(win);
    
    //Calculate resolution and scale
    render.resolution = resolution;
    Vec2<float> ratios = win.size.to<float>() / resolution.to<float>();
    render.scale = (ratios.x < ratios.y) ? floor(ratios.x) : floor(ratios.y);
    
    //V-Sync
    render.vsync = vsync;
    
    return render;
}

void Renderer::draw(const TextureData &tex, DrawID draw_id) {
    const DrawData* data = getDrawData(draw_id);
    if (data == nullptr)
        log::error("Tried to draw an object with a null pointer draw data");
    draw_queue[&tex].push_back(data);
}

void Renderer::update() {
    static int frame = 0;
    log::info("Frame: %d", frame);
    frame++;
    
    for (const auto &[tex, draw] : draw_queue) {
        for (const auto &data : draw) {
            log::info("Indices: %d", data->index_size);
        }
    }
    
    draw_queue.clear();
}

void Renderer::test(WindowData &win, RenderData &render) {
    static DrawID rect_draw_id = createDrawData(rect_vertices, rect_indices);
    
    TextureData tex{};
    Renderer::draw(tex, rect_draw_id);
    
    Renderer::update();
    
    API::renderTest(win, render);
}

void Renderer::clean(RenderData &render) {
    API::clean(render.api);
}
