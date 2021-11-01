//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "r_next_renderer.h"
#include "r_window.h"
#include "r_textures.h"

using namespace Verse;
using namespace Graphics;

//TEST
namespace {
    TextureData test_tex;
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
    
    //TEST
    test_tex = Texture::load(render, "res/graphics/texture.jpg", Texture::TEXTURE_CHANNELS_RGBA);
    
    return render;
}

void Renderer::test(WindowData &win, RenderData &render) {
    API::renderTest(win, render);
}

void Renderer::clean(RenderData &render) {
    //TEST
    vkDestroySampler(render.api.device, test_tex.sampler, nullptr);
    vkDestroyImageView(render.api.device, test_tex.image_view, nullptr);
    vkDestroyImage(render.api.device, test_tex.image, nullptr);
    vkFreeMemory(render.api.device, test_tex.memory, nullptr);
    
    API::clean(render.api);
}
