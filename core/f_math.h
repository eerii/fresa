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
    
    //---Linear Algebra---
    /*
     : Coordinate systems
            Local/object space -(model)-> World space -(view)-> View/camera space -(projection)-> Clip space -(viewport)-> Screen space
            Model matrix - scale, rotate and translate (in that order) an object to place it in world space
            View matrix - move the entire scene to be referenced from the camera's point of view
            Projection matrix - normalizes coordinates to be in the range [-1.0, 1.0]
     : Projections
            Orthographic
            Perspective
     */
}
