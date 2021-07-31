//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "ftime.h"

using namespace Verse;

namespace {
    ui32 last_timer_id = 0;
}

ui64 Time::current = 0;
ui64 Time::previous = 0;
double Time::delta = 0;
std::map<ui32, Timer> Time::timers = {};

ui32 Verse::time() {
    return SDL_GetTicks();
}

ui64 Verse::time_precise() {
    return SDL_GetPerformanceCounter();
}

double Verse::time_precise_difference(ui64 t1) {
    return ((double)(time_precise() - t1) / (double)SDL_GetPerformanceFrequency()) * 1000.0;
}

double Verse::time_precise_difference(ui64 t1, ui64 t2) {
    return ((double)(t2 - t1) / (double)SDL_GetPerformanceFrequency()) * 1000.0;
}

ui32 Verse::setTimer(ui32 ms) {
    ui32 timer_id = last_timer_id + 1;
    while (Time::timers.find(timer_id) != Time::timers.end())
        timer_id += 1;
    last_timer_id = timer_id;
    
    Time::timers[timer_id] = Timer(ms, time());
    return timer_id;
}

bool Verse::checkTimer(ui32 timer, float game_speed) {
    if (Time::timers.find(timer) == Time::timers.end())
        return false;
    
    ui32 curr = time();
    Time::timers[timer].current += (float)(curr - Time::timers[timer].previous) * game_speed;
    Time::timers[timer].previous = curr;
    
    bool done = Time::timers[timer].current >= Time::timers[timer].duration;
    if (done)
        Time::timers.erase(timer);
    
    return done;
}

ui32 Verse::getTimerRemainder(ui32 timer) {
    if (Time::timers.find(timer) == Time::timers.end())
        return 0;
    
    return Time::timers[timer].duration - Time::timers[timer].current;
}

void Verse::stopTimer(ui32 timer) {
    Time::timers.erase(timer);
}
