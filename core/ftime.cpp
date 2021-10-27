//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "ftime.h"

using namespace Verse;

namespace {
    TimerID last_timer_id = 0;
}

Clock::time_point Time::current = {};
Clock::time_point Time::previous = {};
Clock::time_point Time::next = {};

std::map<TimerID, Timer> Time::timers = {};

Clock::time_point Verse::time() {
    return Clock::now();
}

TimerID Verse::setTimer(ui32 ms) {
    TimerID timer_id = last_timer_id + 1;
    
    while (Time::timers.find(timer_id) != Time::timers.end())
        timer_id++;
    last_timer_id = timer_id;
    
    Time::timers[timer_id] = Timer(std::chrono::milliseconds(ms), time());
    
    return timer_id;
}

bool Verse::checkTimer(TimerID timer, float game_speed) {
    if (Time::timers.find(timer) == Time::timers.end())
        return false;
    
    Clock::time_point current = time();
    Duration delta = current - Time::timers[timer].start;
    
    bool done = delta >= Time::timers[timer].duration;
    if (done)
        Time::timers.erase(timer);
    
    return done;
}

Duration Verse::getTimerRemainder(TimerID timer) {
    if (Time::timers.find(timer) == Time::timers.end())
        return Duration(0);
    
    Clock::time_point current = time();
    Duration delta = current - Time::timers[timer].start;
    
    return Time::timers[timer].duration - delta;
}

void Verse::stopTimer(TimerID timer) {
    Time::timers.erase(timer);
}

double Verse::ns(Duration duration) {
    return duration.count();
}

double Verse::ms(Duration duration) {
    return duration.count() * 1.0e-6;
}

double Verse::sec(Duration duration) {
    return duration.count() * 1.0e-9;
}
