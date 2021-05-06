//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "input.h"

using namespace Verse;
using namespace Input;

namespace {
    InputState g_last_state;
    InputState g_curr_state;
    InputState g_next_state;
    InputState g_empty_state;
}


void Input::init() {
    g_last_state = g_empty_state;
    g_curr_state = g_empty_state;
    g_next_state = g_empty_state;
}

void Input::frame() {
    g_last_state = g_curr_state;
    g_curr_state = g_next_state;
    
    {
        for (int i = 0; i < max_keyboard_keys; i++) {
            g_next_state.keyboard.pressed[i] = false;
            g_next_state.keyboard.released[i] = false;
        }
        
        for (int i = 0; i < max_mouse_buttons; i++) {
            g_next_state.mouse.pressed[i] = false;
            g_next_state.mouse.released[i] = false;
        }
    }
    g_next_state.mouse.wheel = 0;
}


void Input::onMouseMove(float x, float y) {
    g_next_state.mouse.position.x = x;
    g_next_state.mouse.position.y = y;
}

void Input::onMouseScreenMove(float x, float y)
{
    g_next_state.mouse.screen_position.x = x;
    g_next_state.mouse.screen_position.y = y;
}

void Input::onMouseDown(ui8 button)
{
    if (button >= 0 && button < max_mouse_buttons)
    {
        g_next_state.mouse.down[button] = true;
        g_next_state.mouse.pressed[button] = true;
    }
}

void Input::onMouseUp(ui8 button)
{
    if (button >= 0 && button < max_mouse_buttons)
    {
        g_next_state.mouse.down[button] = false;
        g_next_state.mouse.released[button] = true;
    }
}

void Input::onMouseWheel(int p)
{
    g_next_state.mouse.wheel = p;
}


void Input::onKeyDown(Key key)
{
    int i = (int)key;
    if (i >= 0 && i < max_keyboard_keys)
    {
        g_next_state.keyboard.down[i] = true;
        g_next_state.keyboard.pressed[i] = true;
    }
}

void Input::onKeyUp(Key key)
{
    int i = (int)key;
    if (i >= 0 && i < max_keyboard_keys)
    {
        g_next_state.keyboard.down[i] = false;
        g_next_state.keyboard.released[i] = true;
    }
}


const InputState* Input::state() { return &g_curr_state; }
const InputState* Input::lastState() { return &g_last_state; }


Vec2f Input::mouse() { return g_curr_state.mouse.position; }
Vec2f Input::mouseScreen() { return g_curr_state.mouse.screen_position; }

int Input::mouseWheel() { return g_curr_state.mouse.wheel; };


bool Input::pressed(ui8 button) {
    return button >= 0 && button < max_mouse_buttons && g_curr_state.mouse.pressed[button];
}
bool Input::down(ui8 button) {
    return button >= 0 && button < max_mouse_buttons && g_curr_state.mouse.down[button];
}
bool Input::released(ui8 button) {
    return button >= 0 && button < max_mouse_buttons && g_curr_state.mouse.released[button];
}


bool Input::pressed(Key key) {
    int i = (int)key;
    return i > 0 && i < max_keyboard_keys && g_curr_state.keyboard.pressed[i];
}
bool Input::down(Key key) {
    int i = (int)key;
    return i > 0 && i < max_keyboard_keys && g_curr_state.keyboard.down[i];
}
bool Input::released(Key key) {
    int i = (int)key;
    return i > 0 && i < max_keyboard_keys && g_curr_state.keyboard.released[i];
}


bool Input::ctrl() { return Input::down(Key::LeftControl) || Input::down(Key::RightControl); }
bool Input::shift() { return Input::down(Key::LeftShift) || Input::down(Key::RightShift); }
bool Input::alt() { return Input::down(Key::LeftAlt) || Input::down(Key::RightAlt); }

const char* Input::nameOf(Key key)
{
    switch (key)
    {
        #define DEFINE_KEY(name, value) case Key::name: return #name;
        VERSE_KEY_DEFINITIONS
        #undef DEFINE_KEY
    }

    return "Unknown";
}
