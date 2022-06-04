//: fresa by jose pazos perez, licensed under GPLv3
#include "f_time.h"

using namespace Fresa;

Clock::time_point Fresa::time() {
    //---The actual time ^·^---
    return Clock::now();
}

TimerID Fresa::setTimer(ui32 ms) {
    //---Set timer---
    //      Creates a new timer and appends it to the timer list, saves the duration and start time
    static TimerID timer_id = 0;
    do timer_id++;
        while (Time::timers.find(timer_id) != Time::timers.end());
    
    Time::timers[timer_id] = Timer(std::chrono::milliseconds(ms), time());
    
    return timer_id;
}

bool Fresa::checkTimer(TimerID timer, float game_speed) {
    //---Check timer---
    //      Returns true when the timer is finished, and then removes it from the list
    //      It will return false if the timer is not registered (For example, when it is called multiple times after a timer is finished)
    if (Time::timers.find(timer) == Time::timers.end())
        return false;
    
    Clock::time_point current = time();
    Duration delta = current - Time::timers[timer].start;
    
    bool done = delta >= Time::timers[timer].duration;
    if (done)
        Time::timers.erase(timer);
    
    return done;
}

Duration Fresa::getTimerRemainder(TimerID timer) {
    //---Timer remainder---
    //      Returns the time for the timer to finish
    if (Time::timers.find(timer) == Time::timers.end())
        return Duration(0);
    
    Clock::time_point current = time();
    Duration delta = current - Time::timers[timer].start;
    
    return Time::timers[timer].duration - delta;
}

void Fresa::stopTimer(TimerID timer) {
    //---Unregister timer---
    Time::timers.erase(timer);
}

double Fresa::ns(Duration duration) {
    //---Duration in nanoseconds---
    return duration.count();
}

double Fresa::ms(Duration duration) {
    //---Duration in milliseconds---
    return duration.count() * 1.0e-6;
}

double Fresa::sec(Duration duration) {
    //---Duration in seconds---
    return duration.count() * 1.0e-9;
}

void Time::updateCallbackTimers() {
    std::vector<TimerID> to_erase{};
    for (auto &[timer, callback] : callback_timers) {
        if (checkTimer(timer)) {
            callback();
            to_erase.push_back(timer);
        }
    }
    for (auto &timer : to_erase) //: In order to avoid problems with the previous for
        callback_timers.erase(timer);
}
