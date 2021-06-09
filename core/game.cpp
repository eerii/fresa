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
    ui16 fps_time;
    ui16 frames;

    ui64 physics_time;
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
    Graphics::init(c);
    
#ifndef DISABLE_GUI
    Gui::init(c);
#endif
    
    physics_time = time_precise();
    
    return true;
}

bool Game::update(Config &c) {
    timeFrame();
    
    //CHECK SCENE
    if (c.active_scene == nullptr) {
        log::error("FATAL: SCENE NOT DEFINED");
        return false;
    }
    
    //PHYSICS UPDATE
    c.physics_delta = TIMESTEP * c.timestep_modifier * 0.001f * c.game_speed;
    if (not physicsUpdate(c))
        return false;
   
    
    //RENDER UPDATE
    Graphics::render(c);
    
    //PREVENT RUNNING TOO FAST
    ui16 frame_ticks = (ui16)time_precise_difference(Time::current);
    if (frame_ticks <= 1000.0 / (float)Graphics::getRefreshRate())
        SDL_Delay((1000.0 / (float)Graphics::getRefreshRate()) - frame_ticks);
    
    //FPS
    frames++;
    fps_time += (ui16)time_precise_difference(Time::current);
    if (fps_time > 200) {
        c.fps = round((float)frames / (float)(fps_time * 0.001f));
        frames = 0;
        fps_time = 0;
    }
    
    return true;
}

bool Game::physicsUpdate(Config &c) {
    ui64 time_before_physics = time_precise();
    while (accumulator >= TIMESTEP * c.timestep_modifier) {
        accumulator -= TIMESTEP * c.timestep_modifier;
        
        //GET EVENTS
        if (not Events::handleEvents(c))
            return false;
        
#ifndef DISABLE_GUI
        //UPDATE GUI
        if(c.enable_gui)
            Gui::update(c);
#endif
        
        //UPDATE SYSTEMS
        PHYSICS_UPDATE_SYSTEMS
        
        //PREPARE FOR NEXT INPUT
        Input::frame();
    }
    c.physics_time = time_precise_difference(time_before_physics);
    
    return true;
}

void Game::timeFrame() {
    Time::previous = Time::current;
    Time::current = time_precise();
    Time::delta = time_precise_difference(Time::previous, Time::current);
    accumulator += Time::delta;
    if (accumulator > 10000)
        accumulator = 0;
}

void Game::stop() {
    log::debug("Closing the game...");
    
    Graphics::destroy();
    SDL_Quit();
}
