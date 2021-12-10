//project fresa, 2017-2022
//by jose pazos perez
//all rights reserved uwu

#include "game.h"

#include "log.h"

#include "input.h"
#include "file.h"
#include "events.h"
#include "f_time.h"

#include "gui.h"
#include "r_graphics.h"

#include "system_list.h"

#include <thread>

using namespace Fresa;

namespace
{
    bool is_paused = false;
}

bool Game::init(Config &c) {
    //---Game setup---
    log::debug("Starting the game...");
    
    //: File system
    //  (only working for macos at the moment)
    File::init();

    //: SDL
    SDL_version version;
    SDL_GetVersion(&version);
    log::debug("SDL v%d.%d.%d", version.major, version.minor, version.patch);
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) != 0) {
        log::error("SDL_Init has failed!!", SDL_GetError());
        return false;
    }
    
    //: Graphics
    if (not Graphics::init())
        return false;
    
    return true;
}

bool Game::update(Config &c) {
    //---Game update---
    
    //: Check if paused
    while (is_paused) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        Events::EventTypes e = Events::handleEvents(c);
        if (e == Events::EVENT_QUIT)
            return false;
        if (e == Events::EVENT_CONTINUE)
            is_paused = false;
    }
    
    //: Check scene
    if (not scene_list.count(active_scene)) {
        log::error("Scene not defined!");
        return false;
    }
    
    //: Physics update
    if (not physicsUpdate(c))
        return false;
   
    //: Render update
    Graphics::update();
    
    //: Advance time
    timeFrame(c);
    
    return true;
}

bool Game::physicsUpdate(Config &c) {
    //---Physics update---
    //      Here the "physics" processes (movement, collisions, input...) get calculated at a fixed timestep
    //      This is achieved using an accumulator that will get filled with the time passed each frame, and then time will be discounted from it
    //      in regular intervals, updating the physics then. We can use that to calculate the delta time.
    //      (for reference check https://gafferongames.com/post/fix_your_timestep/)
    
    //: Physics time calculation
    Clock::time_point time_before_physics = time();
    
    while (Time::accumulator >= Conf::timestep * 1.0e6) {
        //: One phisics iteration time calculation
        Clock::time_point time_before_physics_iteration = time();
        
        //: Timestep
        Time::accumulator -= Conf::timestep * 1.0e6; //: In nanoseconds
        Time::physics_delta = Conf::timestep * 1.0e-3 * Conf::game_speed; //: In seconds
        
        //: Events
        Events::EventTypes e = Events::handleEvents(c);
        if (e == Events::EVENT_QUIT)
            return false;
        if (e == Events::EVENT_PAUSE)
            is_paused = true;
        
        //: GUI
        #ifndef DISABLE_GUI
        Gui::update(c);
        #endif
        
        //: Systems
        System::physicsUpdateSystems();
        
        //: Input
        Input::frame();
        
        Performance::one_physics_iteration_time = ms(time() - time_before_physics_iteration);
    }
    
    Performance::physics_time = ms(time() - time_before_physics);
    
    return true;
}

void Game::timeFrame(Config &c) {
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
}

void Game::stop() {
    //---Clean resources---
    log::debug("Closing the game...");
    
    Graphics::stop();
    SDL_Quit();
}
