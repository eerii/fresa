//* window
//      handles window context creation and management, as well as the window related datatypes
#pragma once

#include "r_types.h"
#include "fresa_math.h"

namespace fresa::graphics
{
    // ·············
    // · DATATYPES ·
    // ·············

    //: window type
    //      contains the main window reference and the relevant properties
    struct Window {
        GLFWwindow* window;
        GLFWmonitor* monitor;
        Vec2<ui16> size;
    };

    // ···········
    // · SYSTEMS ·
    // ···········

    namespace window
    {
        //: initialize window
        Window create();

        //: resize callback (implemented on r_*_api.cpp)
        void onResize(GLFWwindow* window, int width, int height);

        //: get current monitor
        GLFWmonitor* getMonitor();

        //: get dpi
        float getDPI();

        //: get refresh rate
        ui16 getRefreshRate();
    }
}