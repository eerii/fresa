//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"
#include <map>
#include <chrono>

namespace Verse
{
    using Clock = std::chrono::steady_clock;
    using Duration = std::chrono::nanoseconds;
    using TimerID = ui32;

    struct Timer {
        Clock::time_point start;
        Duration duration;
        
        Timer() = default;
        Timer(Duration p_d, Clock::time_point p_s) : duration(p_d), start(p_s) {};
    };

    struct Time {
        static Clock::time_point current;
        static Clock::time_point previous;
        static Duration delta;
        
        static Clock::time_point current_render;
        static Clock::time_point previous_render;
        static Duration delta_render;
        
        static std::map<TimerID, Timer> timers;
    };

    Clock::time_point time();
    
    TimerID setTimer(ui32 ms);
    bool checkTimer(TimerID timer, float game_speed = 1.0f);
    Duration getTimerRemainder(TimerID timer);
    void stopTimer(TimerID timer);

    double ns(Duration duration);
    double ms(Duration duration);
    double sec(Duration duration);
}
