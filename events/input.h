//: fresa by jose pazos perez, licensed under GPLv3
#pragma once

#include <set>

#include "types.h"
#include "events.h"
#include "log.h"

namespace Fresa
{
    namespace Input
    {
        //---Keyboard---
        using Key = ui32;
        
        //: Events
        inline Event::Event<Key> event_key_down;
        inline Event::Event<Key> event_key_up;
        
        //: State
        struct KeyboardState {
            std::set<Key> pressed;
            std::set<Key> down;
            std::set<Key> released;
        };
        
        inline KeyboardState keyboard{};
        inline KeyboardState keyboard_next{};
        
        //: Functions
        inline bool key_pressed(Key key) { return keyboard.pressed.count(key); }
        inline bool key_down(Key key) { return keyboard.down.count(key); }
        inline bool key_released(Key key) { return keyboard.released.count(key); }
        
        //---Mouse---
        enum class MouseButton {
            Left = SDL_BUTTON_LEFT,
            Middle = SDL_BUTTON_MIDDLE,
            Right = SDL_BUTTON_RIGHT,
        };
        
        //: Events
        inline Event::Event<Vec2<>> event_mouse_move;
        inline Event::Event<int> event_mouse_wheel;
        inline Event::Event<MouseButton> event_mouse_down;
        inline Event::Event<MouseButton> event_mouse_up;
        
        //: State
        struct MouseState {
            Vec2<int> position;
            int wheel;
            
            std::set<MouseButton> pressed;
            std::set<MouseButton> down;
            std::set<MouseButton> released;
        };

        inline MouseState mouse{};
        inline MouseState mouse_next{};
        
        //: Functions
        inline bool mouse_pressed(MouseButton button) { return mouse.pressed.count(button); }
        inline bool mouse_down(MouseButton button) { return mouse.down.count(button); }
        inline bool mouse_released(MouseButton button) { return mouse.released.count(button); }
        
        //---Initialization---
        inline void init() {
            //: Keyboard events
            event_key_down.callback([](const Key &key){
                keyboard_next.pressed.insert(key);
                keyboard_next.down.insert(key);
            });
            event_key_up.callback([](const Key &key){
                keyboard_next.released.insert(key);
                keyboard_next.down.erase(key);
            });
            
            //: Mouse events
            event_mouse_move.callback([](const Vec2<> &pos){
                mouse_next.position = pos;
            });
            event_mouse_wheel.callback([](const int &wh){
                mouse_next.wheel = wh;
            });
            event_mouse_down.callback([](const MouseButton &button){
                mouse_next.pressed.insert(button);
                mouse_next.down.insert(button);
            });
            event_mouse_up.callback([](const MouseButton &button){
                mouse_next.released.insert(button);
                mouse_next.down.erase(button);
            });
        }
        
        //---Frame---
        inline void frame() {
            keyboard = keyboard_next;
            keyboard_next.pressed.clear();
            keyboard_next.released.clear();
            
            mouse = mouse_next;
            mouse_next.pressed.clear();
            mouse_next.released.clear();
        }
        
        //---Key name conversions---
        inline str getKeyName(Key key) {
            str name = str(SDL_GetKeyName((SDL_Keycode)key));
            if (name == "")
                log::error("Couldn't get name from key (%d) : %s", key, SDL_GetError());
            return name;
        }
        
        inline Key getKeyFromName(str name) {
            Key key = Key(SDL_GetKeyFromName(name.c_str()));
            if (key == SDLK_UNKNOWN)
                log::error("Couldn't get key from name (%s) : %s", name.c_str(), SDL_GetError());
            return key;
        }
        
        //---GUI---
        #ifndef DISABLE_GUI
        inline bool gui_using_keyboard = false;
        inline bool gui_using_mouse = false;
        #endif
    }
}
