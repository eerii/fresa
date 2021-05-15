//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "config.h"

namespace Verse::Graphics::Window
{
    SDL_Window* createWindow(Config &c);
    void onResize(SDL_Event &e, Config &c);
    void updateVsync(Config &c);
}
