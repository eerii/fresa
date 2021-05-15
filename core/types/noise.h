//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"
#include "log.h"

#include "perlin_noise.h"

#include <math.h>

namespace Verse::Math
{
    void whiteNoise(ui16 size, ui8 dim, ui8* noise_data);
    void perlinNoise(Vec2 size, Vec2 offset, float freq, ui8 octaves, ui32 seed, ui8* noise_data);
}
