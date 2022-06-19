//* fresa_time
//      time management system, based on std::chrono
#pragma once

#include "fresa_types.h"
#include <chrono>

//* use literals for specifying time duration, for example, 10s or 3ms
using namespace std::literals::chrono_literals;

namespace fresa
{
    //* std::chrono clock
    using clock = std::chrono::steady_clock;

    //* get current time
    inline clock::time_point time() { return clock::now(); }

    //* fixed delta time for updates, new simulation each 1/N seconds
    auto constexpr update_frequency = 100;
    auto constexpr dt = std::chrono::duration<ui64, std::ratio<1, update_frequency>>(1);
}