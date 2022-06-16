//* engine

#include "engine.h"
#include "log.h"
#include "fresa_time.h"
#include "unit_test.h"
#include "system.h"

using namespace fresa;

constexpr std::array<ui8, 3> version = {0, 4, 1};

//* main entry point
//      called from main, it creates the engine, runs it and closes it when finished
void fresa::run(int argv, char** args) {
    log::info("{} {}.{}.{}", fmt::format(fmt::emphasis::bold, "fresa"), version[0], version[1], version[2]);

    //: command line arguments
    auto arguments = detail::handle_arguments(argv, args);

    //: run tests if requested
    //      tests can be specified using command line arguments as "-t test_a,test_b"
    auto test_it = arguments.find("t");
    test_it == arguments.end() ? test_runner.run({""}) : test_runner.run(split(test_it->second, ',') | ranges::to_vector);

    //: initialization
    fresa::detail::init();

    //: update loop
    bool running = true;
    while (running) {
        running = fresa::detail::update();
    }

    //: cleanup
    fresa::detail::stop();
    log::info("bye! c:");
}

//------------------------------------------------------------------------------

//* initialization
void fresa::detail::init() {
    log::debug("setting up the engine");

    //- register systems ...
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
    system::stop();
}

//* create argument list
//      parses the command line arguments and creates a list of key-value pairs
//      arguments are passed in the form of "-k value"
std::unordered_map<str, str> fresa::detail::handle_arguments(int argv, char** args) {
    std::unordered_map<str, str> arguments;
    for (int i = 1; i < argv; i++) {
        auto a = str(args[i]);
        auto len = a.size();
        if (a[0] == '-') {
            str key = a.substr(1, 1);
            arguments[key] = len > 2 ? a.substr(2, len - 2) : "";
            continue;
        }
        if (args[i-1][0] == '-') {
            str key = str(args[i-1]).substr(1, 1);
            arguments[key] = a;
            continue;
        };
        log::error("Invalid argument '{}'", a);
    }
    return arguments;
}