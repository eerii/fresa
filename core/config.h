//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#include "types.h"

namespace Fresa::Config
{
    inline const str name = "Proxecto Fresa";
    inline const ui8 version[3] = {0, 3, 7};
    inline const Vec2<ui32> window_size = {1024, 720};
    inline const Vec2<ui32> resolution = {256, 180};
    
    inline const float timestep = 10.0f;
    inline float game_speed = 1.0f;
}
