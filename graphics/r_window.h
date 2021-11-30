//project fresa, 2017-2022
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "config.h"
#include "r_dtypes.h"

namespace Fresa::Graphics::Window
{
    WindowData create(Vec2<> size, str name);
    ui16 getRefreshRate(WindowData &win);

    Vec2<float> sceneToWindow(Config &c, Vec2<> s_pos);
    Vec2<> windowToScene(Config &c, Vec2<float> w_pos);
}
