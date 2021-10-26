//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "r_vulkan_core.h"
#include "r_opengl_api.h"

namespace Verse::Graphics
{
    #if defined USE_VULKAN
        using GraphicsAPI = Vulkan;
    #elif defined USE_OPENGL
        using GraphicsAPI = OpenGL;
    #endif

    struct RenderData {
        GraphicsAPI api;
        Vec2<> resolution;
        ui16 scale;
        bool vsync;
    };
}
