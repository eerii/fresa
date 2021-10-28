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
#include "r_graphics.h"
#include "system_list.h"

#include <thread>

using namespace Verse;

namespace
{
    double accumulator;
    double fps_time;
    ui32 frames;
    Clock::duration fps_limit;
    bool is_paused = false;
}

bool Game::init(Config &c) {
    log::debug("Starting the game...");
    
    //INITIALIZE FILE SYSTEM
    File::init();

    //INITIALIZE SDL
    SDL_version version;
    SDL_GetVersion(&version);
    log::debug("SDL v%d.%d.%d", version.major, version.minor, version.patch);
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) != 0) {
        log::error("SDL_Init has failed!!", SDL_GetError());
        return false;
    }
    
    //INITIALIZE GRAPHICS
    if (not Graphics::init())
        return false;
    
    //INITIALIZE TIME
    Time::current = time();
    fps_limit = round<Clock::duration>(std::chrono::duration<double>{1./60.});
    Time::next = Time::current + fps_limit;
    
    return true;
}

bool Game::update(Config &c) {
    //PAUSED GAME
    while (is_paused) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        Events::EventTypes e = Events::handleEvents(c);
        if (e == Events::EVENT_QUIT)
            return false;
        if (e == Events::EVENT_CONTINUE)
            is_paused = false;
    }
    
    //CHECK SCENE
    if (c.active_scene == nullptr) {
        log::error("FATAL: SCENE NOT DEFINED");
        return false;
    }
    
    //PHYSICS UPDATE
    if (not physicsUpdate(c))
        return false;
   
    //RENDER UPDATE
    Graphics::update();
    
    //TIME FRAME
    timeFrame(c);
    
    return true;
}

bool Game::physicsUpdate(Config &c) {
    while (accumulator >= c.timestep * 1.0e6) {
        accumulator -= c.timestep * 1.0e6;
        c.physics_delta = c.timestep * 1.0e-3 * (double)c.game_speed;
        
        //GET EVENTS
        Events::EventTypes e = Events::handleEvents(c);
        if (e == Events::EVENT_QUIT)
            return false;
        if (e == Events::EVENT_PAUSE)
            is_paused = true;
        
#ifndef DISABLE_GUI
        //UPDATE GUI
        Gui::update(c);
#endif
        
        //UPDATE SYSTEMS
        System::physicsUpdateSystems(c);
        
        //PREPARE FOR NEXT INPUT
        Input::frame();
    }
    
    c.physics_interpolation = accumulator / c.timestep;
    
    return true;
}

void Game::timeFrame(Config &c) {
    if (time() < Time::next)
        std::this_thread::sleep_until(Time::next);
    else if (time() - Time::next > 10 * fps_limit)
        Time::next = time();
    
    frames++;
    fps_time += ns(Time::next - Time::current);
    
    if (fps_time > 1.0e9) {
        log::info("FPS - %d", (int)round(frames * (1.0e9 / fps_time)));
        frames = 0; fps_time = 0;
    }
    
    Time::previous = Time::current;
    Time::current = Time::next;
    Time::next = Time::current + fps_limit;
    
    accumulator += ns(Time::current - Time::previous);
    if (accumulator > 1.0e10)
        accumulator = 0;
}

void Game::stop() {
    log::debug("Closing the game...");
    
    Graphics::stop();
    SDL_Quit();
}
