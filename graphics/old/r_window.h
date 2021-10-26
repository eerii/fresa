//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "config.h"

namespace Verse::Graphics
{
    struct WindowInfo {
        SDL_Window* window;
        Vec2<> size;
        ui16 refresh_rate;
    };

    namespace Window
    {
        WindowInfo createWindow(int size_x, int size_y, str name);
        ui16 getRefreshRate(WindowInfo &win_info);
    
        void onResize(SDL_Event &e, Config &c);
        void updateVsync(bool use_vsync);

        Vec2<float> sceneToWindow(Config &c, Vec2<> s_pos);
        Vec2<> windowToScene(Config &c, Vec2<float> w_pos);

        
    }
}
