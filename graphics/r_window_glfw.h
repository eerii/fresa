//* window_glfw
//      this defines the datatypes and functions used to interact with the windowing api
//      included by r_window.h as a glfw implementation of the windowing api
#pragma once

//: glfw windowing library
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace fresa::graphics
{
    // ···············
    // · DEFINITIONS ·
    // ···············

    //: interfaces for glfw objects
    using IWindow = GLFWwindow;
    using IMonitor = GLFWmonitor;
}