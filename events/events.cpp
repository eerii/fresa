//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#include "events.h"

#include "input.h"
#include "gui.h"
#include "r_window.h"

#include "log.h"

using namespace Fresa;

//---Event handling---
//      Here all the system events are processed with SDL built in event system, and they are translated to fresa's event system

void Events::handleSystemEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                event_quit.publish();
            case SDL_KEYDOWN:
                if (event.key.repeat == 0)
                    Input::event_key_down.publish((Input::Key)event.key.keysym.sym);
                break;
            case SDL_KEYUP:
                if (event.key.repeat == 0)
                    Input::event_key_up.publish((Input::Key)event.key.keysym.sym);
                break;
            case SDL_MOUSEMOTION:
                Input::event_mouse_move.publish(Vec2<float>(event.motion.x, event.motion.y)); //This event consumes 5-10% of CPU, check in the future
                break;
            case SDL_MOUSEBUTTONDOWN:
                Input::event_mouse_down.publish((Input::MouseButton)event.button.button);
                break;
            case SDL_MOUSEBUTTONUP:
                Input::event_mouse_up.publish((Input::MouseButton)event.button.button);
                break;
            case SDL_MOUSEWHEEL:
                Input::event_mouse_wheel.publish(event.wheel.y);
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
                    event_paused.publish(true);
                if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
                    event_paused.publish(false);
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                    Graphics::Window::event_window_resize.publish(Vec2<>(event.window.data1, event.window.data2));
                break;
            case SDL_USEREVENT:
                bool* done = reinterpret_cast<bool*>(event.user.data1);
                *done = true;
                break;
        }
    }
}
