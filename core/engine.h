//* engine
//      handles initialization, update and stop
//      central point of the engine, calls all the other subsystems
#pragma once

namespace fresa
{
    //* call from main.cpp
    void run();

    //* can be called when the engine needs to be stopped
    void quit();
    void force_quit();

    //* different stages of the engine, referenced from run()
    namespace detail {
        void init();
        void update();
        void stop();
    }
}