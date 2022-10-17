//* rendering_api
//      the main entrypoint of the graphics engine
#pragma once

//: rendering implementation
#include "r_api_vulkan.h"

//: spirv-cross, shader reflection and cross compilation
#include "spirv_glsl.hpp"
namespace spv_c = spirv_cross;

#include "system.h"

namespace fresa::graphics
{
    // ···················
    // · GRAPHICS SYSTEM ·
    // ···················
    //      this is included in the system manager and initializes the graphics api
    //      it will also run in the graphics update game loop and render the scene
    struct GraphicsSystem {
        inline static System<GraphicsSystem, system::SystemPriority::GRAPHICS> system;
        //: defined on r_api_*.cpp
        static void init();
        static void update();
        static void stop();
    };

    // ·······················
    // · GRAPHICS API OBJECT ·
    // ·······················
    //      holds information specific to the graphics api currently rendering (vk, gl, ...)
    //      right now only a vulkan implementation is provided
    using GraphicsAPI = vk::VulkanAPI;
    inline std::unique_ptr<const GraphicsAPI> api;
}