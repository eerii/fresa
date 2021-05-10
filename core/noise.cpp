//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "noise.h"
#include "time.h"

using namespace Verse;

void Math::whiteNoise(ui16 size, ui8 dim, ui8* noise_data) {
    ui32 len = pow(size, dim);
    
    for (int i = 0; i < len; i++) {
        noise_data[i] = (ui8)(rand() % 255);
    }
}

void Math::perlinNoise(Vec2 size, Vec2 offset, float freq, ui8 octaves, ui32 seed, ui8 *noise_data) {
    Math::PerlinNoise perlin(seed);
    
    float fx = (float)(size.x + 1) / freq;
    float fy = (float)size.y / freq;
    
    int i = 0;
    for (ui32 y = offset.y; y < offset.y + size.y; y++) {
        for (ui32 x = offset.x; x < offset.x + (size.x + 1); x++) {
            noise_data[i++] = (ui8)round(perlin.accumulatedOctaveNoise2D_0_1(x / fx, y / fy, octaves) * 255.0f);
        }
    }
}
