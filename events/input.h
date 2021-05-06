//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

//thanks to noelfb and his engine (blah)[https://github.com/NoelFB/blah] for the help with his part

#pragma once

#include <SDL2/SDL.h>

#include "dtypes.h"
#include "log.h"

#include "imgui.h"

#define VERSE_KEY_DEFINITIONS \
    DEFINE_KEY(Unknown, 0) \
    DEFINE_KEY(A, 4) \
    DEFINE_KEY(B, 5) \
    DEFINE_KEY(C, 6) \
    DEFINE_KEY(D, 7) \
    DEFINE_KEY(E, 8) \
    DEFINE_KEY(F, 9) \
    DEFINE_KEY(G, 10) \
    DEFINE_KEY(H, 11) \
    DEFINE_KEY(I, 12) \
    DEFINE_KEY(J, 13) \
    DEFINE_KEY(K, 14) \
    DEFINE_KEY(L, 15) \
    DEFINE_KEY(M, 16) \
    DEFINE_KEY(N, 17) \
    DEFINE_KEY(O, 18) \
    DEFINE_KEY(P, 19) \
    DEFINE_KEY(Q, 20) \
    DEFINE_KEY(R, 21) \
    DEFINE_KEY(S, 22) \
    DEFINE_KEY(T, 23) \
    DEFINE_KEY(U, 24) \
    DEFINE_KEY(V, 25) \
    DEFINE_KEY(W, 26) \
    DEFINE_KEY(X, 27) \
    DEFINE_KEY(Y, 28) \
    DEFINE_KEY(Z, 29) \
    DEFINE_KEY(D1, 30) \
    DEFINE_KEY(D2, 31) \
    DEFINE_KEY(D3, 32) \
    DEFINE_KEY(D4, 33) \
    DEFINE_KEY(D5, 34) \
    DEFINE_KEY(D6, 35) \
    DEFINE_KEY(D7, 36) \
    DEFINE_KEY(D8, 37) \
    DEFINE_KEY(D9, 38) \
    DEFINE_KEY(D0, 39) \
    DEFINE_KEY(Enter, 40) \
    DEFINE_KEY(Escape, 41) \
    DEFINE_KEY(Backspace, 42) \
    DEFINE_KEY(Tab, 43) \
    DEFINE_KEY(Space, 44) \
    DEFINE_KEY(Minus, 45) \
    DEFINE_KEY(Equals, 46) \
    DEFINE_KEY(LeftBracket, 47) \
    DEFINE_KEY(RightBracket, 48) \
    DEFINE_KEY(BackSlash, 49) \
    DEFINE_KEY(NonUSHash, 50) \
    DEFINE_KEY(Semicolon, 51) \
    DEFINE_KEY(Apostrophe, 52) \
    DEFINE_KEY(Grave, 53) \
    DEFINE_KEY(Comma, 54) \
    DEFINE_KEY(Period, 55) \
    DEFINE_KEY(Slash, 56) \
    DEFINE_KEY(Capslock, 57) \
    DEFINE_KEY(F1, 58) \
    DEFINE_KEY(F2, 59) \
    DEFINE_KEY(F3, 60) \
    DEFINE_KEY(F4, 61) \
    DEFINE_KEY(F5, 62) \
    DEFINE_KEY(F6, 63) \
    DEFINE_KEY(F7, 64) \
    DEFINE_KEY(F8, 65) \
    DEFINE_KEY(F9, 66) \
    DEFINE_KEY(F10, 67) \
    DEFINE_KEY(F11, 68) \
    DEFINE_KEY(F12, 69) \
    DEFINE_KEY(PrintScreen, 70) \
    DEFINE_KEY(ScrollLock, 71) \
    DEFINE_KEY(Pause, 72) \
    DEFINE_KEY(Insert, 73) \
    DEFINE_KEY(Home, 74) \
    DEFINE_KEY(PageUp, 75) \
    DEFINE_KEY(Delete, 76) \
    DEFINE_KEY(End, 77) \
    DEFINE_KEY(PageDown, 78) \
    DEFINE_KEY(Right, 79) \
    DEFINE_KEY(Left, 80) \
    DEFINE_KEY(Down, 81) \
    DEFINE_KEY(Up, 82) \
    DEFINE_KEY(NumlockClear, 83) \
    DEFINE_KEY(KP_Divide, 84) \
    DEFINE_KEY(KP_Multiply, 85) \
    DEFINE_KEY(KP_Minus, 86) \
    DEFINE_KEY(KP_Plus, 87) \
    DEFINE_KEY(KP_Enter, 88) \
    DEFINE_KEY(KP_1, 89) \
    DEFINE_KEY(KP_2, 90) \
    DEFINE_KEY(KP_3, 91) \
    DEFINE_KEY(KP_4, 92) \
    DEFINE_KEY(KP_5, 93) \
    DEFINE_KEY(KP_6, 94) \
    DEFINE_KEY(KP_7, 95) \
    DEFINE_KEY(KP_8, 96) \
    DEFINE_KEY(KP_9, 97) \
    DEFINE_KEY(KP_0, 98) \
    DEFINE_KEY(KP_Period, 99) \
    DEFINE_KEY(NonUSBackSlash, 100) \
    DEFINE_KEY(Application, 101) \
    DEFINE_KEY(Power, 102) \
    DEFINE_KEY(KP_Equals, 103) \
    DEFINE_KEY(F13, 104) \
    DEFINE_KEY(F14, 105) \
    DEFINE_KEY(F15, 106) \
    DEFINE_KEY(F16, 107) \
    DEFINE_KEY(F17, 108) \
    DEFINE_KEY(F18, 109) \
    DEFINE_KEY(F19, 110) \
    DEFINE_KEY(F20, 111) \
    DEFINE_KEY(F21, 112) \
    DEFINE_KEY(F22, 113) \
    DEFINE_KEY(F23, 114) \
    DEFINE_KEY(F24, 115) \
    DEFINE_KEY(Execute, 116) \
    DEFINE_KEY(Help, 117) \
    DEFINE_KEY(Menu, 118) \
    DEFINE_KEY(Select, 119) \
    DEFINE_KEY(Stop, 120) \
    DEFINE_KEY(Again, 121) \
    DEFINE_KEY(Undo, 122) \
    DEFINE_KEY(Cut, 123) \
    DEFINE_KEY(Copy, 124) \
    DEFINE_KEY(Paste, 125) \
    DEFINE_KEY(Find, 126) \
    DEFINE_KEY(Mute, 127) \
    DEFINE_KEY(VolumeUp, 128) \
    DEFINE_KEY(VolumeDown, 129) \
    DEFINE_KEY(KP_Comma, 133) \
    DEFINE_KEY(KP_EqualsAs400, 134) \
    DEFINE_KEY(International1, 135) \
    DEFINE_KEY(International2, 136) \
    DEFINE_KEY(International3, 137) \
    DEFINE_KEY(International4, 138) \
    DEFINE_KEY(International5, 139) \
    DEFINE_KEY(International6, 140) \
    DEFINE_KEY(International7, 141) \
    DEFINE_KEY(International8, 142) \
    DEFINE_KEY(International9, 143) \
    DEFINE_KEY(Language1, 144) \
    DEFINE_KEY(Language2, 145) \
    DEFINE_KEY(Language3, 146) \
    DEFINE_KEY(Language4, 147) \
    DEFINE_KEY(Language5, 148) \
    DEFINE_KEY(Language6, 149) \
    DEFINE_KEY(Language7, 150) \
    DEFINE_KEY(Language8, 151) \
    DEFINE_KEY(Language9, 152) \
    DEFINE_KEY(AltErase, 153) \
    DEFINE_KEY(SysReq, 154) \
    DEFINE_KEY(Cancel, 155) \
    DEFINE_KEY(clear, 156) \
    DEFINE_KEY(Prior, 157) \
    DEFINE_KEY(Return2, 158) \
    DEFINE_KEY(Separator, 159) \
    DEFINE_KEY(Out, 160) \
    DEFINE_KEY(Oper, 161) \
    DEFINE_KEY(ClearAgain, 162) \
    DEFINE_KEY(CRSEL, 163) \
    DEFINE_KEY(EXSEL, 164) \
    DEFINE_KEY(KP_00, 176) \
    DEFINE_KEY(KP_000, 177) \
    DEFINE_KEY(ThousandsSeparator, 178) \
    DEFINE_KEY(DecimalSeparator, 179) \
    DEFINE_KEY(CurrencyUnit, 180) \
    DEFINE_KEY(CurrencySubUnit, 181) \
    DEFINE_KEY(KP_LeftParen, 182) \
    DEFINE_KEY(KP_RightParent, 183) \
    DEFINE_KEY(KP_LeftBrace, 184) \
    DEFINE_KEY(KP_RightBrace, 185) \
    DEFINE_KEY(KP_Tab, 186) \
    DEFINE_KEY(KP_BackSpace, 187) \
    DEFINE_KEY(KP_A, 188) \
    DEFINE_KEY(KP_B, 189) \
    DEFINE_KEY(KP_C, 190) \
    DEFINE_KEY(KP_D, 191) \
    DEFINE_KEY(KP_E, 192) \
    DEFINE_KEY(KP_F, 193) \
    DEFINE_KEY(KP_XOR, 194) \
    DEFINE_KEY(KP_Power, 195) \
    DEFINE_KEY(KP_Percent, 196) \
    DEFINE_KEY(KP_Less, 197) \
    DEFINE_KEY(KP_Greater, 198) \
    DEFINE_KEY(KP_Ampersand, 199) \
    DEFINE_KEY(KP_DoubleAmpersand, 200) \
    DEFINE_KEY(KP_VerticalBar, 201) \
    DEFINE_KEY(KP_DoubleVerticalBar, 202) \
    DEFINE_KEY(KP_Colon, 203) \
    DEFINE_KEY(KP_Hash, 204) \
    DEFINE_KEY(KP_Space, 205) \
    DEFINE_KEY(KP_At, 206) \
    DEFINE_KEY(KP_EXCLAM, 207) \
    DEFINE_KEY(KP_MemStore, 208) \
    DEFINE_KEY(KP_MemRecall, 209) \
    DEFINE_KEY(KP_MemClear, 210) \
    DEFINE_KEY(KP_MemAdd, 211) \
    DEFINE_KEY(KP_MemSubstract, 212) \
    DEFINE_KEY(KP_MemMultiply, 213) \
    DEFINE_KEY(KP_MemDivide, 214) \
    DEFINE_KEY(KP_PlusMinus, 215) \
    DEFINE_KEY(KP_Clear, 216) \
    DEFINE_KEY(KP_ClearEntry, 217) \
    DEFINE_KEY(KP_Binary, 218) \
    DEFINE_KEY(KP_Octal, 219) \
    DEFINE_KEY(KP_Decimal, 220) \
    DEFINE_KEY(KP_Hexadecimal, 221) \
    DEFINE_KEY(LeftControl, 224) \
    DEFINE_KEY(LeftShift, 225) \
    DEFINE_KEY(LeftAlt, 226) \
    DEFINE_KEY(LeftGui, 227) \
    DEFINE_KEY(RightControl, 228) \
    DEFINE_KEY(RightShift, 229) \
    DEFINE_KEY(RightAlt, 230) \
    DEFINE_KEY(RightGui, 231)

namespace Verse
{
    namespace Input
    {
        constexpr int max_keyboard_keys = 512;
        constexpr int max_mouse_buttons = 16;
        
        struct KeyboardState {
            bool pressed[Input::max_keyboard_keys];
            bool down[Input::max_keyboard_keys];
            bool released[Input::max_keyboard_keys];
        };

        struct MouseState {
            bool pressed[Input::max_mouse_buttons];
            bool down[Input::max_mouse_buttons];
            bool released[Input::max_mouse_buttons];
            Vec2f screen_position;
            Vec2f position;
            int wheel;
        };

        struct InputState {
            KeyboardState keyboard;
            MouseState mouse;
        };

        enum class Key {
            #define DEFINE_KEY(name, value) name = value,
            VERSE_KEY_DEFINITIONS
            #undef DEFINE_KEY
        };

        enum class MouseButton {
            Left = SDL_BUTTON_LEFT,
            Middle = SDL_BUTTON_MIDDLE,
            Right = SDL_BUTTON_RIGHT,
        };

        //Events

        // This is called internally by the app, and initializes the input state
        void init();

        // This is called internally by the app, and updates the input state
        void frame();

        // Call this when the Mouse moves relative to the window
        void onMouseMove(float x, float y);

        // Call this when the Mouse moves relative to the screen
        void onMouseScreenMove(float x, float y);

        // Call this when a Mouse Button is pressed
        void onMouseDown(ui8 button);

        // Call this when a Mouse Button is released
        void onMouseUp(ui8 button);
    
        // Call this when the Mouse Wheel is changed
        void onMouseWheel(int p);

        // Call this when a keyboard key is pressed
        void onKeyDown(Input::Key key);

        // Call this when a keyboard key is released
        void onKeyUp(Input::Key key);


        //Functions

        // Returns the Input State of the current frame.
        // This pointer is only valid for the current frame and should not be stored.
        const InputState* state();

        // Returns the Input State of the previous frame.
        // This pointer is only valid for the current frame and should not be stored.
        const InputState* lastState();


        // Gets the Mouse Position
        Vec2f mouse();

        // Gets the Mouse Position in Screen Coordinates
        Vec2f mouseScreen();
    
        //Gets the Mouse Wheel
        int mouseWheel();


        // Checks if the given Mouse Button is pressed
        bool pressed(ui8 button);
           
        // Checks if the given Mouse Button is down
        bool down(ui8 button);
       
        // Checks if the given Mouse Button is released
        bool released(ui8 button);

        
        // Returns 1 if the keyboard key was pressed this frame, and 0 otherwise.
        bool pressed(Key key);

        // Returns 1 if the keyboard key was held this frame, and 0 otherwise.
        bool down(Key key);

        // Returns 1 if the keyboard key was released this frame, and 0 otherwise.
        bool released(Key key);


        // Checks if the Left or Right Ctrl Key is down
        bool ctrl();

        // Checks if the Left or Right Shift Key is down
        bool shift();

        // Checks if the Left or Right Alt Key is down
        bool alt();


        // Returns a string name of the given key
        const char* nameOf(Key key);

    }
}
