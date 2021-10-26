//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"

namespace Verse::Math
{
    bool checkAABB(Rect2<int> &a, Rect2<int> &b);
    bool checkCircleAABB(Rect2<int> &a, Vec2<int> &pos, float r);
    void perlinNoise(Vec2<int> size, Vec2<int> offset, float freq, int levels, ui8* noise_data, bool reset = false);
    float smoothDamp(float current, float target, float &current_vel, float time, float max_speed, float delta_time);
}
