//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "config.h"
#include "r_windowdata.h"

namespace Verse::Graphics::Window
{
    WindowData create(Vec2<> size, str name);
    ui16 getRefreshRate(WindowData &win);

    void onResize(SDL_Event &e, Config &c);

    Vec2<float> sceneToWindow(Config &c, Vec2<> s_pos);
    Vec2<> windowToScene(Config &c, Vec2<float> w_pos);
}
