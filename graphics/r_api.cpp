//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "r_api.h"

using namespace Verse;
using namespace Graphics;

void API::configure() {
    #if defined USE_VULKAN
        
    #elif defined USE_OPENGL
        GL::configOpenGL();
    #endif
}

GraphicsAPI API::create(WindowData &win) {
    GraphicsAPI api;
    #if defined USE_VULKAN
        api = VK::createVulkan(win);
    #elif defined USE_OPENGL
        api = GL::createOpenGL(win);
    #endif
    return api;
}
