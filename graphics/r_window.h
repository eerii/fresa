//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"
#include "log.h"
#include "config.h"

namespace Verse::Graphics::Window
{
    SDL_Window* createWindow(Config &c);
    void onResize(SDL_Event &e, Config &c);
}
