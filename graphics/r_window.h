//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#include "config.h"
#include "r_dtypes.h"
#include "events.h"

namespace Fresa::Graphics::Window
{
    WindowData create(Vec2<> size, str name);
    ui16 getRefreshRate(WindowData &win, bool force = false);
    
    inline Events::Event<Vec2<>> event_window_resize;
}
