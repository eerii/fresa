//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include <SDL2/SDL.h>

#include "config.h"

namespace Verse::Game
{
    bool init(Config &c);

    bool update(Config &c);
    bool physicsUpdate(Config &c);

    void timeFrame(Config &c);

    void stop();
}
