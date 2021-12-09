//project fresa, 2017-2022
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include <SDL2/SDL.h>

#include "config.h"

namespace Fresa::Game
{
    bool init(Config &c);

    bool update(Config &c);
    bool physicsUpdate(Config &c);

    void timeFrame(Config &c);

    void stop();
}
