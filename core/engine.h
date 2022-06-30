//* engine
//      handles initialization, update and stop
//      central point of the engine, calls all the other subsystems
#pragma once

#include "fresa_types.h"

namespace fresa
{
    //* call from main.cpp
    void run(int argv = 0, char** args = nullptr);

    //* different stages of the engine, referenced from run()
    namespace detail {
        void init();
        bool update();
        void stop();
        std::unordered_map<str, str> handle_arguments(int argv, char** args);
    }
}

