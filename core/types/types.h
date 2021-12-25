//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#include <SDL2/SDL.h>

#include <string>
#include <vector>
#include <array>
#include <map>

#include <functional>
#include <memory>

#include "aliases.h"
#include "vec2.h"
#include "rect2.h"

//: Compile time type name in readable format (https://stackoverflow.com/a/56766138/17575567)
template <typename T>
constexpr auto type_name() {
    std::string_view name, prefix, suffix;
    #ifdef __clang__
    name = __PRETTY_FUNCTION__;
    prefix = "auto type_name() [T = ";
    suffix = "]";
    #elif defined(__GNUC__)
    name = __PRETTY_FUNCTION__;
    prefix = "constexpr auto type_name() [with T = ";
    suffix = "]";
    #elif defined(_MSC_VER)
    name = __FUNCSIG__;
    prefix = "auto __cdecl type_name<";
    suffix = ">(void)";
    #endif
    name.remove_prefix(prefix.size());
    name.remove_suffix(suffix.size());
    return name;
}

#ifndef DISABLE_GUI
#define IF_GUI(x) x
#else
#define IF_GUI(x)
#endif
