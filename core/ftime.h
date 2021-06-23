//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"
#include <map>

namespace Verse
{
    struct Timer {
        float current = 0;
        ui32 previous;
        ui32 duration;
        Timer() = default;
        Timer(ui32 p_d, ui32 p_p) : duration(p_d), previous(p_p) {};
    };

    struct Time {
        static ui64 current;
        static ui64 previous;
        static ui64 delta;
        static std::map<ui32, Timer> timers;
    };

    ui32 time();
    ui64 time_precise();
    float time_precise_difference(ui64 t1);
    float time_precise_difference(ui64 t1, ui64 t2);
    
    ui32 setTimer(ui32 ms);
    bool checkTimer(ui32 timer, float game_speed = 1.0f);
    ui32 getTimerRemainder(ui32 timer);
    void stopTimer(ui32 timer);
}
