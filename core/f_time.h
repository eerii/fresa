//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#include "types.h"
#include <chrono>

namespace Fresa
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
    
    namespace Time {
        //---Time points for update---
        //      Current time is the time() at the start of each update
        //      Previous is the current time of the previous update
        //      Next is used when manually controlling the FPS limit for a more stable approach
        inline Clock::time_point current{};
        inline Clock::time_point previous{};
        inline Clock::time_point next{};
        
        //---Physics---
        inline double physics_delta = 0.0;
        inline double accumulator = 0.0;
        
        //---Timer list---
        inline std::map<TimerID, Timer> timers{};
    }

    Clock::time_point time();
    
    TimerID setTimer(ui32 ms);
    bool checkTimer(TimerID timer, float game_speed = 1.0f);
    Duration getTimerRemainder(TimerID timer);
    void stopTimer(TimerID timer);

    double ns(Duration duration);
    double ms(Duration duration);
    double sec(Duration duration);
    
    namespace Performance {
        //---Counters for performance metrics---
        
        //: One iteration of the accumulator physics loop
        inline double one_physics_iteration_time = 0.0;
        //: The entire frame of physics updates (can be 0 since the physics might not be updated in the frame)
        inline double physics_time = 0.0;
        
        //: Render time
        inline double render_time = 0.0;
    }
}
