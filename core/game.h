//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include <SDL2/SDL.h>

#include "config.h"

#define TIMESTEP 10 //Update the simulation every 10ms (0.01s)
#define DELTA (TIMESTEP * 0.001f)

namespace Verse::Game
{
    bool init(Config &c);

    bool update(Config &c, Scene &s);
    bool physicsUpdate(Config &c, Scene &s);
    void timeFrame();

    void stop();
}
