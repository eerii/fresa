//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#ifndef DISABLE_GUI

#include "config.h"

#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_impl_sdl.h"

#if defined USE_OPENGL
#include "imgui_impl_opengl3.h"
#elif defined USE_VULKAN
#include "imgui_impl_vulkan.h"
#endif

//---WARNING---
//      This is a disastrous and total chaotic file
//      It will be rewritten soon using the new reflection API, please ignore until then

namespace Fresa::Gui
{
    struct ActiveWindows {
        static bool entities;
        static bool test;
        static bool performance;
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
static bool Combo(const char* label, int* curr_index, std::vector<Fresa::str>& values)
{
    if (values.empty()) { return false; }
    return Combo(label, curr_index, vector_getter,
        static_cast<void*>(&values), (int)values.size());
}
[[maybe_unused]]
static bool Combo(const char* label, Fresa::ui8& curr_index, std::vector<Fresa::str>& values) {
    int i = curr_index;
    bool c = Combo(label, &i, values);
    curr_index = i;
    return c;
}

[[maybe_unused]]
static bool ListBox(const char* label, int* currIndex, std::vector<Fresa::str>& values)
{
    if (values.empty()) { return false; }
    return ListBox(label, currIndex, vector_getter,
        static_cast<void*>(&values), (int)values.size());
}

}

#endif
