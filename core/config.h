//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#include "types.h"

namespace Fresa::Config
{
    inline str name = "Proxecto Fresa";
    inline ui8 version[3] = {0, 3, 7};
    inline Vec2<ui32> window_size = {1024, 720};
    inline Vec2<ui32> resolution = {256, 180};
    
    inline const float timestep = 10.0f;
    inline float game_speed = 1.0f;
}
