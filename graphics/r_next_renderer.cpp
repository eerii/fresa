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
    std::vector<VertexData> rect_vertices = {
        {{-1.f,  1.f,  0.f}, {0.f, 1.f, 0.f}},
        {{ 1.f,  1.f,  0.f}, {1.f, 1.f, 0.f}},
        {{-1.f, -1.f,  0.f}, {0.f, 0.f, 0.f}},
        {{ 1.f, -1.f,  0.f}, {1.f, 0.f, 0.f}},
    };
    std::vector<ui16> rect_indices = {
        0, 1, 2, 2, 3, 0
    };

    const std::vector<VertexData> vertices = {
        {{-1.f, -1.f, -1.f}, {0.701f, 0.839f, 0.976f}}, //Light
        {{1.f, -1.f, -1.f}, {0.117f, 0.784f, 0.596f}}, //Teal
        {{1.f, 1.f, -1.f}, {1.000f, 0.815f, 0.019f}}, //Yellow
        {{-1.f, 1.f, -1.f}, {0.988f, 0.521f, 0.113f}}, //Orange
        {{-1.f, -1.f, 1.f}, {0.925f, 0.254f, 0.345f}}, //Red
        {{1.f, -1.f, 1.f}, {0.925f, 0.235f, 0.647f}}, //Pink
        {{1.f, 1.f, 1.f}, {0.658f, 0.180f, 0.898f}}, //Purple
        {{-1.f, 1.f, 1.f}, {0.258f, 0.376f, 0.941f}}, //Blue
    };

    const std::vector<ui16> indices = {
        0, 3, 1, 3, 2, 1,
        1, 2, 5, 2, 6, 5,
        4, 7, 0, 7, 3, 0,
        3, 7, 2, 7, 6, 2,
        4, 0, 5, 0, 1, 5,
        5, 6, 4, 6, 7, 4,
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

void Renderer::test(WindowData &win, RenderData &render) {
    //Draw something for test
    static DrawID test_draw_id = API::registerDrawData(render.api, vertices, indices);
    static TextureData test_texture_data{};
    API::draw(test_texture_data, test_draw_id);
    
    //Do some tests
    for (const auto &[tex, draw] : API::draw_queue) {
        for (const auto &data : draw) {
            API::renderTest(win, render, data);
        }
    }
    API::draw_queue.clear();
}

void Renderer::clean(RenderData &render) {
    API::clean(render.api);
}
