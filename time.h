//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"

namespace Verse
{
    struct Time {
        static ui64 current;
        static ui64 previous;
        static ui64 delta;
    };

    ui64 time();
    
}
