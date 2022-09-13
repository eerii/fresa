//* window
//      handles window creation and management

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
        .window = w,
        .size = config.window_size
    };
}

void graphics::resizeWindow(ui16 width, ui16 height) {
    if (width == 0 or height == 0) return;
    graphics_assert(win != nullptr, "window not initialized");

    auto w = const_cast<Window*>(win.get());
    w->size = Vec2<ui16>{width, height};

    log::graphics("window resized to {}x{}", width, height);
}