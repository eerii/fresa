//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "fmath.h"

#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"

#include "log.h"

using namespace Verse;

void Math::perlinNoise(Vec2 size, Vec2 offset, float freq, int levels, ui8 *noise_data) {
    Vec2 fixed_size = size;
    if (fixed_size.x % 4 != 0)
        fixed_size.x = fixed_size.x - fixed_size.x % 4 + 4;
    if (fixed_size.y % 4 != 0)
        fixed_size.y = fixed_size.y - fixed_size.y % 4 + 4;
    
    float fx = (float)fixed_size.x / freq;
    float fy = (float)fixed_size.y / freq;
    
    for (ui32 y = 0; y < size.y; y++) {
        for (ui32 x = 0; x < size.x; x++) {
            float n = stb_perlin_noise3((x + offset.x) / fx, (y + offset.y) / fy, 0.0f, 0, 0, 0);
            if (levels > 0 and levels < 16) {
                for (ui8 l = 1; l <= levels; l++) {
                    float m = stb_perlin_noise3(pow(2.0f, l) * (x + offset.x) / fx, pow(2.0f, l) * (y + offset.y) / fy, 0.0f, 0, 0, 0);
                    n += m / pow(2.0f, l);
                }
            }
            noise_data[x + y*fixed_size.x] = (ui8)round((n + 1.0f) * 127.5f); //TODO: It maybe works differently for web, check it
        }
    }
}

bool Math::checkAABB(Rect2 &a, Rect2 &b) {
    return (*a.x < *b.x + *b.w) and
           (*a.x + *a.w > *b.x) and
           (*a.y < *b.y + *b.h) and
           (*a.y + *a.h > *b.y);
}
