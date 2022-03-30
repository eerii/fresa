//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3

#pragma once

#include "r_types.h"
#include "events.h"
#include "reflection.h"

namespace Fresa::Graphics
{
    //---------------------------------------------------
    //: Data
    //---------------------------------------------------
    
    //: Window data, contains all the relevant state of the current window
    //      If the window is resized, it will be reconstructed using the onResize() callback
    //      It is not meant to be modified after it is created
    inline struct WindowData {
        SDL_Window* window;
        str name;
        Vec2<ui16> size;
        float dpi;
        ui16 refresh_rate;
        bool vsync;
        Event::Observer resize_observer;
    } window;
    
    //: Window transform uniform, used for binding scaled projections with a window shader
    struct WindowTransform {
        Members(WindowTransform, model, projection)
        glm::mat4 model;
        glm::mat4 projection;
    };
    
    //: Resize event, triggered by events.cpp whenever the window is resized
    inline Event::Event<Vec2<ui16>> event_window_resize;
    
    //---------------------------------------------------
    //: Systems
    //---------------------------------------------------
    namespace Window {
        //: Creates a new WindowData object
        WindowData create(Vec2<ui16> size, str name, WindowData* previous = nullptr);
        
        //: Obtains the display DPI of the current monitor
        float getDPI(SDL_Window* win);
        
        //: Obtains the refresh rate of the current monitor
        ui16 getRefreshRate(SDL_Window* win);
        
        //: Returns a WindowTransform that applies a transformation that scales the resolution the closest to the window size
        //: For example, if the resolution is 100x50, and the window is 720x480, it will scale it to 700x350
        WindowTransform getScaledTransform(Vec2<ui16> resolution);
        
        //: Callback for when the window is resized
        void onResize(Vec2<ui16> size);
    }
}
