//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include <SDL2/SDL.h>

#include "config.h"

namespace Verse::Graphics
{
    void init(Config &c);

    void render(Scene &scene, Config &c);
    void clear(Config &c);
    void display(Config &c);

    void destroy();

    void calculateRefreshRate();
    int getRefreshRate();
}
