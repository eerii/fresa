//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "events.h"

#include "input.h"
#include "gui.h"
#include "r_window.h"

#include "log.h"

using namespace Verse;

Events::EventTypes Events::handleEvents(Config &c) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                return EVENT_QUIT;
            case SDL_KEYDOWN:
                if (event.key.repeat == 0) {
                    Input::onKeyDown((Input::Key)event.key.keysym.scancode);
#ifndef DISABLE_GUI
                    Gui::addInputKey(event.key.keysym.sym);
#endif
                }
                break;
            case SDL_KEYUP:
                if (event.key.repeat == 0)
                    Input::onKeyUp((Input::Key)event.key.keysym.scancode);
                break;
            case SDL_MOUSEMOTION:
                Input::onMouseMove(event.motion.x, event.motion.y); //This event consumes 5-10% of CPU, check in the future
                break;
            case SDL_MOUSEBUTTONDOWN:
                Input::onMouseDown(event.button.button);
                break;
            case SDL_MOUSEBUTTONUP:
                Input::onMouseUp(event.button.button);
                break;
            case SDL_MOUSEWHEEL:
                Input::onMouseWheel(event.wheel.y);
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
                    return EVENT_PAUSE;
                if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
                    return EVENT_CONTINUE;
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                    Graphics::Window::onResize(event, c);
                break;
            case SDL_USEREVENT:
                bool* done = reinterpret_cast<bool*>(event.user.data1);
                *done = true;
                break;
        }
    }
    return EVENT_NONE;
}
