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
    
    float fx = (float)size.x / freq;
    float fy = (float)size.y / freq;
    
    for (ui32 y = 0; y <= size.y; y++) {
        for (ui32 x = 0; x < size.x; x++) {
            noise_data[x + y*size.x] = (ui8)round(perlin.accumulatedOctaveNoise2D_0_1((float)(x + offset.y - offset.x) / fx, (float)(y + offset.y - offset.x) / fy, octaves) * 255.0f);
        }
    }
}
