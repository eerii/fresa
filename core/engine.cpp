//* engine

#include "engine.h"

#include "std_types.h"

#include "log.h"
#include "unit_test.h"

#include "fresa_time.h"
#include "fresa_config.h"
#include "system.h"
#include "file.h"

#include "r_api.h"

using namespace fresa;

namespace {
    //: controls wether the engine is running or not
    bool running = true;
}

//* main entry point
//      called from main, it creates the engine and runs it
void fresa::run() {
    log::info("{} {}.{}.{}",
              fmt::format(fmt::emphasis::bold, engine_config.name()),
              engine_config.version()[0], engine_config.version()[1], engine_config.version()[2]);

    //: run tests if requested
    #ifdef FRESA_ENABLE_TESTS
    if constexpr (engine_config.run_tests().size() > 0)
        test_runner.run(split(engine_config.run_tests(), ',') | ranges::to_vector);
    #endif

    //: initialization
    fresa::detail::init();

    //: update loop
    while (running)
        fresa::detail::update();

    //: cleanup and exit
    fresa::detail::stop();
}

//* quit the engine
void fresa::quit() {
    running = false;
}

//* force quit the engine (does not complete the update loop)
void fresa::force_quit() {
    fresa::detail::stop();
    std::abort();
}

//---

//* initialization
void fresa::detail::init() {
    log::debug("setting up the engine");

    //: system initialization
    system::init();
}

//* update
//      update loop implementation based on https://gafferongames.com/post/fix_your_timestep/
//      the simulation updates is decoupled from the frame time, and is instead updated in discrete steps dt
void fresa::detail::update() {
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

    //: cap the frame difference to avoid 'the spiral of death'
    if (frame_time > 250ms) frame_time = 250ms;

    //: assign the new values to the previous time and the accumulator
    previous_time = new_time;
    accumulator += frame_time;

    //: sleep to avoid wasting CPU time
    if (fresa::dt > accumulator + engine_config.game_loop_wait())
        std::this_thread::sleep_for(engine_config.game_loop_wait());

    //: update the simulation with discrete steps
    while (accumulator >= dt) {
        system::update(); //: simulation(t, dt)

        accumulator -= dt;
        simulation_time += dt;
    }

    //? interpolation
    //      const double alpha = std::chrono::duration<double>{accumulator} / dt;
    //      state = current * alpha + previous * (1.0 - alpha);
}

//* stop and cleanup
void fresa::detail::stop() {
    log::debug("closing all systems");
    system::stop();
}