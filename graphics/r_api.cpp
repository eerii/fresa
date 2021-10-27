//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "r_api.h"

using namespace Verse;
using namespace Graphics;

void API::configure() {
    #if defined USE_VULKAN
        
    #elif defined USE_OPENGL
        GL::config();
    #endif
}

GraphicsAPI API::create(WindowData &win) {
    GraphicsAPI api;
    #if defined USE_VULKAN
        api = VK::create(win);
    #elif defined USE_OPENGL
        api = GL::create(win);
    #endif
    return api;
}

void API::renderTest(WindowData &win, RenderData &render) {
    #if defined USE_VULKAN
        render.api.renderFrame(win);
    #elif defined USE_OPENGL
        GL::renderTest(win, render);
    #endif
}
