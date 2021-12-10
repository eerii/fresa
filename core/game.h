//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#include <SDL2/SDL.h>

#include "config.h"

namespace Fresa::Game
{
    bool init();

    bool update();
    bool physicsUpdate();

    void timeFrame();

    void stop();
}
