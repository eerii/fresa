//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "r_vulkan.h"
#include "r_opengl.h"

#include "r_drawdata.h"
#include <map>

namespace Verse::Graphics
{
    struct RenderData {
        Vec2<> resolution;
        ui16 scale;
        bool vsync;
    };
}
