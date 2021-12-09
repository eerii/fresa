//project fresa, 2017-2022
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"

namespace Fresa::Math
{
    bool checkAABB(Rect2<> &a, Rect2<> &b);
    bool checkCircleAABB(Rect2<> &a, Vec2<> &pos, float r);
    void perlinNoise(Vec2<> size, Vec2<> offset, float freq, int levels, ui8* noise_data, bool reset = false);
    float smoothDamp(float current, float target, float &current_vel, float time, float max_speed, float delta_time);
    float to_sRGB(float c);
    float to_linear(float c);
}
