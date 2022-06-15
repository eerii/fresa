//* engine

#include "engine.h"
#include "log.h"
#include "fresa_test.h"
#include "fresa_time.h"

using namespace fresa;

constexpr std::array<ui8, 3> version = {0, 4, 0};

//* main entry point
void fresa::run() {
    log::debug("{} {}.{}.{}", fmt::format(fmt::emphasis::bold, "fresa"), version[0], version[1], version[2]);

    //: initialization
    fresa::detail::init();

    //: update loop
    bool running = true;
    while (running) {
        running = fresa::detail::update();
    }

    //: cleanup
    fresa::detail::stop();
    log::debug("bye! c:");
}

//* initialization
void fresa::detail::init() {
    log::debug("setting up the engine");

    //: run tests if requested
    test_runner.run();

    //...
}

//* update
bool fresa::detail::update() {
    //...
    
    return false;
}

//* stop and cleanup
void fresa::detail::stop() {
    log::debug("closing all systems");

    //...
}