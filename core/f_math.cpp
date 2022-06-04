//: fresa by jose pazos perez, licensed under GPLv3
#include "f_math.h"

#include <algorithm>
#include "log.h"

using namespace Fresa;

bool Math::checkAABB(Rect2<> &a, Rect2<> &b) {
    //---Check intersection between two Rect2 (Axis aligned bounding boxes)---
    return (a.x < b.x + b.w) and
           (a.x + a.w > b.x) and
           (a.y < b.y + b.h) and
           (a.y + a.h > b.y);
}

bool Math::checkCircleAABB(Rect2<> &a, Vec2<> &pos, float r) {
    //---Check intersection between a Rect2 and a circle---
    Vec2 dist = Vec2(abs(pos.x - (a.x+a.w/2)), abs(pos.y - (a.y+a.h/2)));
    
    if (dist.x > (a.w/2 + r) or dist.y > (a.h/2 + r))
        return false;
    if (dist.x <= a.w/2 or dist.y <= a.h/2)
        return true;
    
    float corner = pow(dist.x - a.w/2,2) + pow(dist.y - a.h/2,2);
    return (corner <= pow(r,2));
}

float Math::smoothDamp(float current, float target, float &current_vel, float time, float max_speed, float delta_time) {
    //---Smooth damp implementation---
    //      Smoothly transition a value from it's current position to a target one
    //      Spring-damper function that doesn't overshoot. Useful for camera following
    float smooth_time = std::max(0.0001f, time);
    float omega = 2.0f / smooth_time;
    
    float x = omega * delta_time;
    float exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);
    float change = current - target;
    
    float max_change = max_speed * smooth_time;
    change = std::clamp(change, -max_change, max_change);
    if (abs(change) < 2.0f)
        change = 0;
    
    float new_target = current - change;
    
    float temp = (current_vel + omega * change) * delta_time;
    current_vel = (current_vel - omega * temp) * exp;
    float output = new_target + (change + temp) * exp;
    
    if (target - current > 0.0F == output > target) {
        output = target;
        current_vel = (output - target) / delta_time;
    }
    
    return output;
}
