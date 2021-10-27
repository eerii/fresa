//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "config.h"

namespace Verse::Events
{
    enum EventTypes {
        EVENT_QUIT,
        EVENT_PAUSE,
        EVENT_CONTINUE,
        EVENT_NONE
    };

    EventTypes handleEvents(Config &c);
}
