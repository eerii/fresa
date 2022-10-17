//* window
//      handles window context creation and management, as well as the window related datatypes
#pragma once

//: windowing implementation
#include "r_window_glfw.h"

#include "std_types.h"
#include "fresa_math.h"

namespace fresa::graphics
{
    // ···············
    // · DEFINITIONS ·
    // ···············

    //: check for required interface definitions
    static_assert(requires { typename IWindow; },   "windowing api interface 'IWindow' not defined");
    static_assert(requires { typename IMonitor; },  "windowing api interface 'IMonitor' not defined");

    // ·············
    // · DATATYPES ·
    // ·············

    //: window type
    //      contains the window and monitor references, as well as its size (not the framebuffer size)
    struct Window {
        IWindow* window = nullptr;
        IMonitor* monitor = nullptr;
        Vec2<ui16> size = {0, 0};
    };
    inline std::unique_ptr<const Window> win;

    // ···········
    // · SYSTEMS ·
    // ···········

    namespace window
    {
        //: initialize window and callbacks
        Window create();

        //: resize callback (implemented on r_api_*.cpp)
        void onResize(IWindow* window, int width, int height);

        //: get current monitor
        //      this is different from glfwGetPrimaryMonitor() because it is calculated based on the window position
        //      should be more precise if the window is moved to another monitor which may not be the primary one
        //      the monitor information is used to get important data such as the dpi or the refresh rate
        IMonitor* getMonitor(IWindow* window);

        //: get refresh rate (in Hz)
        int getRefreshRate();
    }
}