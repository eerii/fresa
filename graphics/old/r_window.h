//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "config.h"

namespace Verse::Graphics
{
    struct WindowInfo {
        SDL_Window* window;
    };

    namespace Window
    {
        SDL_Window* createWindow(Config &c);
        void onResize(SDL_Event &e, Config &c);
        void updateVsync(Config &c);

        Vec2<float> sceneToWindow(Config &c, Vec2<> s_pos);
        Vec2<> windowToScene(Config &c, Vec2<float> w_pos);

        void calculateRefreshRate(Config &c);
    }
}
