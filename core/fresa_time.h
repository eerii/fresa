//* fresa_time
//      time management system, based on std::chrono

#include "fresa_types.h"
#include <chrono>

using namespace std::literals::chrono_literals;

namespace fresa
{
    //* std::chrono clock
    using clock = std::chrono::steady_clock;

    //* get current time
    inline clock::time_point time() { return clock::now(); }
}