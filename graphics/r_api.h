//* rendering_api
//      the main entrypoint of the graphics engine, includes the barebones libraries for creating a window
#pragma once

#include "r_vulkan.h"
#include "r_window.h"

#include "engine.h"
#include "system.h"
#include "source_loc.h"

namespace fresa::graphics
{
    // ················
    // · GRAPHICS API ·
    // ················

    //* graphics api object
    //      holds information specific to the graphics api currently rendering (vk, gl, ...)
    using GraphicsAPI = VulkanAPI;
    inline std::unique_ptr<const GraphicsAPI> api;

    //* graphics system
    struct GraphicsSystem {
        inline static System<GraphicsSystem, system::SystemPriority::FIRST> system;
        static void init();
        static void update();
        static void stop();
    };

    //: window object
    inline std::unique_ptr<const Window> win;

    // ···········
    // · SHADERS ·
    // ···········

    //* shader types
    enum struct ShaderType {
        VERTEX,
        FRAGMENT,
        COMPUTE
    };

    //* shader extensions
    constexpr auto shader_extensions = std::to_array<str_view>({ ".vert", ".frag", ".comp" });

    //* read spirv code
    std::vector<ui32> readSPIRV(str name, ShaderType type);

    // ···············
    // · EXTRA TOOLS ·
    // ···············
}