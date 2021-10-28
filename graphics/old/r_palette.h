//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "config.h"

namespace Verse::Graphics::Palette
{
    void render(Config &c, ui32 &palette_tex, ui8 &pid);
    void switchPalette(Config &c);
    void setPaletteInterval(int w);
}
