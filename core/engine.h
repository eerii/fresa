//* engine
//      handles initialization, update and stop
//      central point of the engine, calls all the other subsystems
#pragma once

namespace fresa
{
    //* call from main.cpp
    void run();

    //* different stages of the engine, referenced from run()
    namespace detail {
        void init();
        bool update();
        void stop();
    }

    //* user configurable initalization and stop callbacks
    //      note: virtual calls are acceptable here since this is only called once, not every frame
    struct EngineCallback {
        virtual void on_init() const {}; //: called at the end of init()
        virtual void on_stop() const {}; //: called at the start of stop()
    };
}

//* an engine callback file can be specified using the directive FRESA_ENGINE_FILE, it defaults to "game.h"
#ifndef FRESA_ENGINE_FILE
#define FRESA_ENGINE_FILE "game.h"
#endif
