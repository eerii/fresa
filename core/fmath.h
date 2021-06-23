//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"

namespace Verse::Math
{
    void perlinNoise(Vec2 size, Vec2 offset, float freq, int levels, ui8* noise_data);

    bool checkAABB(Rect2 &a, Rect2 &b);
}
