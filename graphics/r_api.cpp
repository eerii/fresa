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

void API::renderTest(WindowData &win, RenderData &render) {
    #if defined USE_VULKAN
        render.api.renderFrame(win);
    #elif defined USE_OPENGL
        glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        SDL_GL_SwapWindow(win.window);
    #endif
}
