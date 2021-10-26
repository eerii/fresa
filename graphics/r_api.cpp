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

void API::init(GraphicsAPI *api, WindowData &win) {
    #if defined USE_VULKAN
        VK::initVulkan(api, win);
    #elif defined USE_OPENGL
        GL::initOpenGL(api, win);
    #endif
}
