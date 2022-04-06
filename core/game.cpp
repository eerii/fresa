//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#include "game.h"

#include "log.h"

#include "input.h"
#include "file.h"
#include "audio.h"
#include "events.h"
#include "scene.h"
#include "f_time.h"

#include "r_graphics.h"

#include <thread>

using namespace Fresa;

namespace {
    bool is_paused = false;
    Event::Observer pausedObserver = Event::event_paused.createObserver([](const bool paused){ is_paused = paused; });
    bool is_quitting = false;
    Event::Observer quitObserver = Event::event_quit.createObserver([](){ is_quitting = true; });
}

bool Game::init() {
    //---Game setup---
    log::debug("Starting the game...");
    
    //: File system
    File::init();

    //: SDL
    SDL_version version;
    SDL_GetVersion(&version);
    log::debug("SDL v%d.%d.%d", version.major, version.minor, version.patch);
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) != 0) {
        log::error("SDL_Init has failed!!", SDL_GetError());
        return false;
    }
    
    //: Graphics
    if (not Graphics::init())
        return false;
    
    //: Input
    Input::init();
    
    //: Audio
    Audio::init();
    
    //: System init
    for (auto &[priority, system] : System::init_systems)
        system.second();
    
    return true;
}

bool Game::update() {
    //---Game update---
    
    //: Check if paused
    while (is_paused) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        Event::handleSystemEvents();
        if (is_quitting) return false;
    }
    
    //: Check scene
    if (not scene_list.count(active_scene)) {
        log::error("Scene not defined!");
        return false;
    }
    
    //: Physics update
    if (not TIME(Performance::physics_frame_time, physicsUpdate))
        return false;
   
    //: Render update
    Graphics::update();
    
    //: Advance time
    timeFrame();
    
    return true;
}

bool Game::physicsUpdate() {
    //---Physics update---
    //      Here the "physics" processes (movement, collisions, input...) get calculated at a fixed timestep
    //      This is achieved using an accumulator that will get filled with the time passed each frame, and then time will be discounted from it
    //      in regular intervals, updating the physics then. We can use that to calculate the delta time.
    //      (for reference check https://gafferongames.com/post/fix_your_timestep/)
    
    while (Time::accumulator >= Config::timestep * 1.0e6) {
        Clock::time_point time_before_physics_iteration = time();
        
        //: Timestep
        Time::accumulator -= Config::timestep * 1.0e6; //: In nanoseconds
        Time::physics_delta = Config::timestep * 1.0e-3 * Config::game_speed; //: In seconds
        
        //: Events
        TIME(Performance::physics_event_time, Event::handleSystemEvents);
        if (is_quitting) return false;
        
        //: Input
        Input::frame();
        
        //: Systems
        Performance::physics_system_time.clear();
        for (auto &[priority, system] : System::physics_update_systems) {
            Performance::physics_system_time.push_back(0);
            TIME(Performance::physics_system_time.back(), system.second);
        }
        
        Performance::physics_iteration_time = ms(time() - time_before_physics_iteration);
    }
    
    return true;
}

void Game::timeFrame() {
    //---Update time---
    //      We want to cap the framerate at the display's refresh rate for graphics updates, but keep the physics updating at regular intervals,
    //      so we use this method for manually setting an fps limit.
    //      We can rely on OpenGL and Vulkan to use V-Sync and limit the framerate that way, which from my tests works with less stutter.
    //      An option for disabling V-Sync should be added to config, and then commented code that would limit the framerate manually.
    
    /*if (time() < Time::next)
        std::this_thread::sleep_until(Time::next);
    else if (time() - Time::next > 10 * fps_limit)
        Time::next = time();*/
    
    //: Update time points
    Time::previous = Time::current;
    Time::current = time(); //Time::next;
    //Time::next = Time::current + fps_limit;
    //fps_limit = round<Clock::duration>(std::chrono::duration<double>{1./60.});
    
    //: Add to accumulator
    Time::accumulator += ns(Time::current - Time::previous);
    
    //: Prevent too many physics updates to pile up (10 seconds)
    if (Time::accumulator > 1.0e10)
        Time::accumulator = 0.0;
    
    //: Update callback timers
    Time::updateCallbackTimers();
}

void Game::stop() {
    //---Clean resources---
    log::debug("Closing the game...");
    
    Graphics::stop();
    SDL_Quit();
}
