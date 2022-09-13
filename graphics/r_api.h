//* rendering_api
//      the main entrypoint of the graphics engine, includes the barebones libraries for creating a window
#pragma once

#include "r_vulkan.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "std_types.h"
#include "source_loc.h"

#include "engine.h"
#include "system.h"
#include "events.h"

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

    // ··········
    // · WINDOW ·
    // ··········

    //* window type
    //      contains the main window reference and the relevant properties
    struct Window {
        GLFWwindow* window;
        Vec2<ui16> size;
    };
    inline std::unique_ptr<const Window> win;

    //* initialize window
    Window createWindow();

    //* resize callback
    void resizeWindow(ui16 width, ui16 height);

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

    //* graphics assertions
    //      checks for errors, always enabled unlike regular assertions
    //      also, unlike regular assertions, this one will call fresa::quit for a softer exit
    template <typename ... T>
    void graphics_assert(bool condition, fmt::format_string<T...> fs, T&& ...t, const detail::source_location &location = detail::source_location::current()) {
        if (not condition) {
            str_view file_name = location.file_name();
            file_name = file_name.substr(file_name.find_last_of("/") + 1);
            detail::log<"GRAPHICS ERROR", LogLevel::ERROR, fmt::color::red>("({}:{}) {}", file_name, location.line(), fmt::format(fs, std::forward<T>(t)...));
            fresa::force_quit();
        }
    }
}