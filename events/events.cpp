//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "events.h"
#include "input.h"
#include "r_window.h"

using namespace Verse;

namespace {
    SDL_Event event;
}

bool Events::handleEvents(Config &c) {
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                return false;
            case SDL_KEYDOWN:
                if (event.key.repeat == 0)
                    Input::onKeyDown((Input::Key)event.key.keysym.scancode);
                break;
            case SDL_KEYUP:
                if (event.key.repeat == 0)
                    Input::onKeyUp((Input::Key)event.key.keysym.scancode);
                break;
            case SDL_MOUSEMOTION:
                Input::onMouseMove(event.motion.x, event.motion.y);
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
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                    Graphics::Window::onResize(event, c);
                break;
        }
    }
    return true;
}
