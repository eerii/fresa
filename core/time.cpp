//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "time.h"

using namespace Verse;

ui32 Time::current = 0;
ui32 Time::previous = 0;
ui32 Time::delta = 0;
std::map<ui32, bool*> Time::timers = {};

ui32 Verse::time() {
    return SDL_GetTicks();
}

ui64 Verse::time_precise() {
    return SDL_GetPerformanceCounter();
}

float Verse::time_precise_difference(ui64 t1) {
    return ((time_precise() - t1) / (float)SDL_GetPerformanceFrequency()) * 1000.0f;
}

float Verse::time_precise_difference(ui64 t1, ui64 t2) {
    return ((t2 - t1) / (float)SDL_GetPerformanceFrequency()) * 1000.0f;
}

ui32 Verse::setTimer(ui32 ms) {
    auto timer_function = [](ui32 interval, void* param) {
        SDL_Event event;
        SDL_UserEvent user_event;
        
        user_event.type = SDL_USEREVENT;
        user_event.code = 0;
        user_event.data1 = param;
        user_event.data2 = NULL;
        
        event.type = SDL_USEREVENT;
        event.user = user_event;
        
        SDL_PushEvent(&event);
        return(interval);
    };
    ui32 (*timer_function_ptr) (ui32, void*) = timer_function;
    
    bool* done = new bool(false);
    void* done_ptr = reinterpret_cast<void*>(done);
    
    ui32 timer = SDL_AddTimer(ms, timer_function_ptr, done_ptr);
    Time::timers[timer] = done;
    
    return timer;
}

bool Verse::checkTimer(ui32 timer) {
    if (Time::timers.find(timer) == Time::timers.end())
        return false;
    
    bool done = *Time::timers[timer];
    if (done)
        Time::timers.erase(timer);
    
    return done;
}

void Verse::stopTimer(ui32 timer) {
    SDL_RemoveTimer(timer);
    Time::timers.erase(timer);
}
