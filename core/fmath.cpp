//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "fmath.h"

#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"

#include "log.h"

using namespace Verse;

bool Math::checkAABB(Rect2<int> &a, Rect2<int> &b) {
    return (a.x < b.x + b.w) and
           (a.x + a.w > b.x) and
           (a.y < b.y + b.h) and
           (a.y + a.h > b.y);
}

bool Math::checkCircleAABB(Rect2<int> &a, Vec2<int> &pos, float r) {
    Vec2 dist = Vec2(abs(pos.x - (a.x+a.w/2)), abs(pos.y - (a.y+a.h/2)));
    
    if (dist.x > (a.w/2 + r) or dist.y > (a.h/2 + r))
        return false;
    if (dist.x <= a.w/2 or dist.y <= a.h/2)
        return true;
    
    float corner = pow(dist.x - a.w/2,2) + pow(dist.y - a.h/2,2);
    return (corner <= pow(r,2));
}

void Math::perlinNoise(Vec2<int> size, Vec2<int> offset, float freq, int levels, ui8 *noise_data, bool reset) {
    float f = freq * 0.001f;
    
    //TODO: Change to scrolling texture instead of rewrite
    
    for (ui32 y = 0; y < size.y; y++) {
        for (ui32 x = 0; x < size.x; x++) {
            if (not reset and x + offset.x + (y + offset.y)*size.x < size.x * size.y - 1) {
                noise_data[x + y*size.x] = noise_data[x + offset.x + (y + offset.y)*size.x];
                continue;
            }
            
            float n = stb_perlin_noise3((x + offset.x) * f, (y + offset.y) * f, 0.0f, 0, 0, 0);
            for (ui8 l = 1; l <= std::min(std::max(levels, 1), 16); l++) {
                float m = stb_perlin_noise3(pow(2.0f, l) * (x + offset.x) * f, pow(2.0f, l) * (y + offset.y) * f, 0.0f, 0, 0, 0);
                n += m / pow(2.0f, l);
            }
            noise_data[x + y*size.x] = (ui8)round((n + 1.0f) * 127.5f); //TODO: It maybe works differently for web, check it
        }
    }
}

float Math::smoothDamp(float current, float target, float &current_vel, float time, float max_speed, float delta_time) {
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
