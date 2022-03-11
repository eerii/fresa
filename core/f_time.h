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
    
    template <typename Callable, typename... Args>
    auto TIME(double &call_time, Callable &f, const Args&... a) {
        constexpr bool is_void = std::is_same_v<decltype(f(a...)), void> == true;
        #ifdef DEBUG
        Clock::time_point time_before_call = time();
        if constexpr (is_void) {
            f(a...);
            call_time = ms(time() - time_before_call);
        } else {
            auto v = f(a...);
            call_time = ms(time() - time_before_call);
            return v;
        }
        #else
        if constexpr (is_void)
            f(a...);
        else
            return f(a...);
        #endif
    }
    
    namespace Performance {
        //---Counters for performance metrics---
        
        //---PHYSICS---
        
        //: The entire frame of physics updates (can be 0 since the physics might not be updated in the frame)
        inline double physics_frame_time = 0.0;
        
        //: One iteration of the accumulator physics loop
        inline double physics_iteration_time = 0.0;
        
        //: A list of the time of one iteration of each physics system (same order as the system list)
        inline std::vector<double> physics_system_time{};
        
        //: One event handling iteration inside the physics time
        inline double physics_event_time = 0.0;
        
        //: Render frame time (The time of the entire rendering call, including vsync)
        inline double render_frame_time = 0.0;
        
        //: Render time for the draw commands to execute
        inline double render_draw_time = 0.0;
        
        //: Timings of each render system
        inline std::vector<double> render_system_time{};
        
        #ifdef USE_VULKAN
        //: In Vulkan, a value that represents the number of nanoseconds it takes for a timestamp query to be incremented by 1
        //: Additionally, if timestamps are not supported in the current graphics queue it will be set back to 0
        inline float timestamp_period = 0.0f;
        #endif
    }
}
