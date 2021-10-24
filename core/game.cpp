//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "game.h"

#include "log.h"
#include "ftime.h"
#include "input.h"
#include "file.h"
#include "events.h"
#include "gui.h"
#include "r_pipeline.h"
#include "system_list.h"

using namespace Verse;

namespace
{
    double accumulator;
    double fps_time;
    ui32 frames;
}

bool Game::init(Config &c) {
    log::debug("Starting the game...");
    
    //INITIALIZE FILE SYSTEM
    File::init();
    
    #if _WIN32
    SetProcessDPIAware();
    #endif

    //INITIALIZE SDL
    SDL_version version;
    SDL_GetVersion(&version);
    log::debug("SDL v%d.%d.%d", version.major, version.minor, version.patch);
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) != 0) {
        log::error("SDL_Init has failed!!", SDL_GetError());
        return false;
    }
    
    //INITIALIZE GRAPHICS
    return Graphics::init(c);
}

bool Game::update(Config &c) {
    timeFrame(c);
    
    //CHECK SCENE
    if (c.active_scene == nullptr) {
        log::error("FATAL: SCENE NOT DEFINED");
        return false;
    }
    
    //PHYSICS UPDATE
    if (not physicsUpdate(c))
        return false;
   
    //RENDER UPDATE
    if (c.window_size.x != 0 and c.window_size.y != 0)
        Graphics::render(c);
    
    return true;
}

bool Game::physicsUpdate(Config &c) {
    while (accumulator >= c.timestep * 1.0e6) {
        Clock::time_point time_before_physics = time();
        
        accumulator -= c.timestep * 1.0e6;
        c.physics_delta = c.timestep * 1.0e-3 * (double)c.game_speed;
        
        //GET EVENTS
        if (not Events::handleEvents(c))
            return false;
        
#ifndef DISABLE_GUI
        //UPDATE GUI
        Gui::update(c);
#endif
        
        //UPDATE SYSTEMS
        System::physicsUpdateSystems(c);
        
        //PREPARE FOR NEXT INPUT
        Input::frame();
        
        c.physics_time = ns(time() - time_before_physics);
    }
    
    c.physics_interpolation = accumulator / c.timestep;
    
    return true;
}

void Game::timeFrame(Config &c) {
    Time::previous = Time::current;
    Time::current = time();
    Time::delta = Time::current - Time::previous;
    
    accumulator += ns(Time::delta);
    if (accumulator > 1.0e10)
        accumulator = 0;
    
    frames++;
    fps_time += ns(Time::delta);
    
    if (fps_time > 1.0e9) {
        c.fps = floor((1.0e9 * frames) / fps_time);
        frames = 0;
        fps_time = 0;
    }
}

void Game::stop() {
    log::debug("Closing the game...");
    
    Graphics::destroy();
    SDL_Quit();
}
