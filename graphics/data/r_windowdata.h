//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"

namespace Verse::Graphics
{
    struct WindowData {
        SDL_Window* window;
        Vec2<> size;
        ui16 refresh_rate;
    };
}
