//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"
#include <map>

namespace Verse
{
    struct Time {
        static ui32 current;
        static ui32 previous;
        static ui32 delta;
        static std::map<ui32, bool*> timers;
    };

    ui32 time();
    ui64 time_precise();
    float time_precise_difference(ui64 t1);
    float time_precise_difference(ui64 t1, ui64 t2);
    
    ui32 setTimer(ui32 ms);
    bool checkTimer(ui32 timer);
    void stopTimer(ui32 timer);
}
