//project fresa, 2017-2022
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "config.h"

namespace Fresa::Events
{
    enum EventTypes {
        EVENT_QUIT,
        EVENT_PAUSE,
        EVENT_CONTINUE,
        EVENT_NONE
    };

    EventTypes handleEvents();
}
