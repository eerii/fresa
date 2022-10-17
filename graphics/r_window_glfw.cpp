//* window
//      handles window creation and management

#include "r_window.h"

#include "engine.h"
#include "fresa_config.h"
#include "fresa_assert.h"

using namespace fresa;

//* initialize window using glfw
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
        .monitor = window::getMonitor(w),
        .size = config.window_size
    };
}

//* get current monitor
GLFWmonitor* graphics::window::getMonitor(GLFWwindow* window) {
    //: monitor list
    int monitor_count;
    GLFWmonitor** monitors = glfwGetMonitors(&monitor_count);

    //: get window position and size
    Vec2<int> pos, size;
    glfwGetWindowPos(window, &pos.x, &pos.y);
    glfwGetWindowSize(window, &size.x, &size.y);

    //: get closest monitor
    GLFWmonitor* closest = monitors[0];
    ui32 max_area = 0;

    if (monitor_count > 1) {
        for (int i = 0; i < monitor_count; i++) {
            //: get monitor position
            Vec2<int> mpos, msize;
            glfwGetMonitorPos(monitors[i], &mpos.x, &mpos.y);

            //: get monitor size
            const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
            msize = { mode->width, mode->height };

            //: check if the window is inside the monitor
            if (pos.x >= mpos.x and pos.x + size.x <= mpos.x + msize.x and pos.y >= mpos.y and pos.y + size.y <= mpos.y + msize.y) {
                //: calculate area
                ui32 area = (pos.x - mpos.x) * (pos.y - mpos.y);

                //: check if the area is bigger than the previous one
                if (area > max_area) {
                    max_area = area;
                    closest = monitors[i];
                }
            }
        }
    }

    log::graphics("monitor: '{}'", lower(glfwGetMonitorName(closest)));
    return closest;
}

//* get refresh rate
int graphics::window::getRefreshRate() {
    soft_assert(win != nullptr, "window not initialized");
    const GLFWvidmode* mode = glfwGetVideoMode(win->monitor);
    return mode->refreshRate;
}