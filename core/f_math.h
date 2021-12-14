//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#include "types.h"

namespace Fresa::Math
{
    bool checkAABB(Rect2<> &a, Rect2<> &b);
    bool checkCircleAABB(Rect2<> &a, Vec2<> &pos, float r);
    float smoothDamp(float current, float target, float &current_vel, float time, float max_speed, float delta_time);
    
    template <typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
    int sign(T val) {
        return (T(0) < val) - (val < T(0));
    }
}
