//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#include "events.h"

#include "input.h"
#include "gui.h"
#include "r_graphics.h"

#include "log.h"

using namespace Fresa;

//---Event handling---
//      Here all the system events are processed with SDL built in event system, and they are translated to fresa's event system

#ifndef DISABLE_GUI
    #define CHECK_GUI_USING_KEYBOARD if (not Input::gui_using_keyboard)
    #define CHECK_GUI_USING_MOUSE if (not Input::gui_using_mouse)
#else
    #define CHECK_GUI_USING_KEYBOARD
    #define CHECK_GUI_USING_MOUSE
#endif

void Event::handleSystemEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        #ifndef DISABLE_GUI
        ImGui_ImplSDL2_ProcessEvent(&event);
        #endif
        
        switch (event.type) {
            case SDL_QUIT: {
                event_quit.publish();
            } case SDL_KEYDOWN: {
                if (event.key.repeat == 0)
                    CHECK_GUI_USING_KEYBOARD Input::event_key_down.publish((Input::Key)event.key.keysym.sym);
                break;
            } case SDL_KEYUP: {
                if (event.key.repeat == 0)
                    CHECK_GUI_USING_KEYBOARD Input::event_key_up.publish((Input::Key)event.key.keysym.sym);
                break;
            } case SDL_MOUSEMOTION: {
                Vec2<> pos{};
                SDL_GetGlobalMouseState(&pos.x, &pos.y);
                CHECK_GUI_USING_MOUSE Input::event_mouse_move.publish(pos);
                break;
            } case SDL_MOUSEBUTTONDOWN: {
                CHECK_GUI_USING_MOUSE Input::event_mouse_down.publish((Input::MouseButton)event.button.button);
                break;
            } case SDL_MOUSEBUTTONUP: {
                CHECK_GUI_USING_MOUSE Input::event_mouse_up.publish((Input::MouseButton)event.button.button);
                break;
            } case SDL_MOUSEWHEEL: {
                CHECK_GUI_USING_MOUSE Input::event_mouse_wheel.publish(event.wheel.y);
                break;
            } case SDL_WINDOWEVENT: {
                if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
                    event_paused.publish(true);
                if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
                    event_paused.publish(false);
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                    Graphics::event_window_resize.publish(Vec2<ui16>((ui16)event.window.data1, (ui16)event.window.data2));
                break;
            } case SDL_USEREVENT: {
                bool* done = reinterpret_cast<bool*>(event.user.data1);
                *done = true;
                break;
            }
        }
    }
}
