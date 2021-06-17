//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#ifdef __EMSCRIPTEN__
#define DISABLE_GUI
#endif

#ifndef DISABLE_GUI

#include "imgui.h"
#include "imgui_stdlib.h"

#include "config.h"

#include <vector>

namespace Verse::Gui
{
    struct ActiveWindows {
        static bool entities;
        static bool test;
    };

    void init(Config &c);
    void update(Config &c);
    void prerender(Config &c, SDL_Window* window);
    void render();
    void addInputKey(SDL_Keycode k);
}

namespace ImGui
{

static auto vector_getter = [](void* vec, int idx, const char** out_text)
{
    auto& vector = *static_cast<std::vector<std::string>*>(vec);
    if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
    *out_text = vector.at(idx).c_str();
    return true;
};

[[maybe_unused]]
static bool Combo(const char* label, int* curr_index, std::vector<Verse::str>& values)
{
    if (values.empty()) { return false; }
    return Combo(label, curr_index, vector_getter,
        static_cast<void*>(&values), (int)values.size());
}
[[maybe_unused]]
static bool Combo(const char* label, Verse::ui8& curr_index, std::vector<Verse::str>& values) {
    int i = curr_index;
    bool c = Combo(label, &i, values);
    curr_index = i;
    return c;
}

[[maybe_unused]]
static bool ListBox(const char* label, int* currIndex, std::vector<Verse::str>& values)
{
    if (values.empty()) { return false; }
    return ListBox(label, currIndex, vector_getter,
        static_cast<void*>(&values), (int)values.size());
}

}

#endif
