//* engine

#include "engine.h"
#include "log.h"
#include "fresa_test.h"
#include "fresa_time.h"

using namespace fresa;

constexpr std::array<ui8, 3> version = {0, 4, 0};

//* main entry point
//      called from main, it creates the engine, runs it and closes it when finished
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

//------------------------------------------------------------------------------

//* initialization
void fresa::detail::init() {
    log::debug("setting up the engine");

    //: run tests if requested
    test_runner.run();
}

//* update
//      update loop implementation based on https://gafferongames.com/post/fix_your_timestep/
//      the simulation updates is decoupled from the frame time, and is instead updated in discrete steps dt
bool fresa::detail::update() {
    //: high precision duration and time points optimized for our delta time
    using duration = decltype(clock::duration{} + fresa::dt);
    using time_point = std::chrono::time_point<fresa::clock, duration>;

    //: persistent variables
    //      previous_time: time point of the previous frame, used to calculate this frame's time
    //      accumulator: increases with frame time, when it is greater than dt, updates the simulation
    //      simulation_time: simulation time, increases in discrete steps of dt
    static time_point previous_time = time();
    static duration accumulator = 0s;
    static time_point simulation_time{};

    //: get current time, and calculate the time difference between this frame and the previous one
    time_point new_time = time();
    auto frame_time = new_time - previous_time;

    //: cap the frame different to avoid 'the spiral of death'
    if (frame_time > 250ms) frame_time = 250ms;

    //: assign the new values to the previous time and the accumulator
    previous_time = new_time;
    accumulator += frame_time;

    //: update the simulation with discrete steps
    while (accumulator >= dt) {
        //- simulation(t, dt)

        accumulator -= dt;
        simulation_time += dt;

        //: temporary fake exit after 1s for testing
        if (simulation_time.time_since_epoch().count() * 1.0e-6 > 1000)
            return false;
    }

    //? interpolation
    //      const double alpha = std::chrono::duration<double>{accumulator} / dt;
    //      state = current * alpha + previous * (1.0 - alpha);

    return true;
}

//* stop and cleanup
void fresa::detail::stop() {
    log::debug("closing all systems");
}