//* rendering_api
//      the main entrypoint of the graphics engine
#pragma once

#include "r_vulkan.h"
#include "r_window.h"
#include "system.h"

namespace fresa::graphics
{
    // ···················
    // · GRAPHICS SYSTEM ·
    // ···················

    struct GraphicsSystem {
        inline static System<GraphicsSystem, system::SystemPriority::FIRST> system;
        //: defined on r_*_api.cpp
        static void init();
        static void update();
        static void stop();
    };

    // ···········
    // · OBJECTS ·
    // ···········

    //: graphics api object
    //      holds information specific to the graphics api currently rendering (vk, gl, ...)
    using GraphicsAPI = VulkanAPI;
    inline std::unique_ptr<const GraphicsAPI> api;

    //: window object
    //      holds information about the window and its context
    inline std::unique_ptr<const Window> win;
}