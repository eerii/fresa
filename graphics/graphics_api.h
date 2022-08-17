//* graphics_api
//      the main entrypoint of the graphics engine, includes the barebones libraries for creating a window
#pragma once

//* glad2
#include <glad/vulkan.h>

//* glfw3
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

//* graphics system
#include "system.h"

namespace fresa
{
    struct GraphicsSystem {
        //: system registration
        inline static System<GraphicsSystem, system::SYSTEM_PRIORITY_FIRST> system;

        //: initialization
        static void init() noexcept;
    };
}