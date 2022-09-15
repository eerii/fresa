//* window
//      handles window creation and management

#include "r_window.h"
#include "r_api.h"

#include "engine.h"
#include "fresa_config.h"
#include "fresa_assert.h"

using namespace fresa;

//: initialize window using glfw
graphics::Window graphics::window::create() {
    //: window name
    const str name = fmt::format("{} - version {}.{}.{} - vulkan", engine_config.name(), engine_config.version()[0], engine_config.version()[1], engine_config.version()[2]);
    
    //: create window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE); //- temporary while developing the core engine
    GLFWwindow* w = glfwCreateWindow(config.window_size.x, config.window_size.y, name.c_str(), nullptr, nullptr);
    strong_assert(w, "failed to create a window");

    //: window callbacks
    //      on close: quits when the window is closed
    glfwSetWindowCloseCallback(w, [](GLFWwindow* window) { fresa::quit(); });
    //      on resize: resizes the swapchain
    glfwSetWindowSizeCallback(w, window::onResize);

    return Window{
        .window = w,
        .size = config.window_size
    };
}

//: get current monitor
GLFWmonitor* graphics::window::getMonitor() {
    return glfwGetPrimaryMonitor(); //!!!!!!!!!!!!!!!!!
}