//* window
//      - ...

#include "r_api.h"
#include "fresa_config.h"

using namespace fresa;

graphics::Window graphics::createWindow() {
    const str name = fmt::format("{} - version {}.{}.{} - vulkan", engine_config.name(), engine_config.version()[0], engine_config.version()[1], engine_config.version()[2]);
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE); //- temporary while developing the core engine
    GLFWwindow* w = glfwCreateWindow(config.window_size.x, config.window_size.y, name.c_str(), nullptr, nullptr);
    graphics_assert(w, "failed to create a window");

    return Window{
        .window = w
    };
}