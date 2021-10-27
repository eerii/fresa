//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "r_next_renderer.h"
#include "r_window.h"

using namespace Verse;
using namespace Graphics;

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
    API::renderTest(win, render);
}
