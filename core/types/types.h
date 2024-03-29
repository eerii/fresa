//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#include <SDL2/SDL.h>

#include <vector>
#include <array>
#include <map>

#include <functional>
#include <memory>
#include <cstdint>

//: Unsigned int
using ui8 = std::uint8_t;
using ui16 = std::uint16_t;
using ui32 = std::uint32_t;
using ui64 = std::uint64_t;

#include "string_helper.h"
#include "vec2.h"
#include "rect2.h"

//: Compile time checks
template<typename T> struct is_vector { static constexpr bool value=false; };
template<typename A> struct is_vector<std::vector<A>> { static constexpr bool value=true; };

template<typename T> struct is_vec2 { static constexpr bool value=false; };
template<typename A> struct is_vec2<Fresa::Vec2<A>> { static constexpr bool value=true; };

template<typename T> struct is_rect2 { static constexpr bool value=false; };
template<typename A> struct is_rect2<Fresa::Rect2<A>> { static constexpr bool value=true; };

//: Compile time type name in readable format (https://stackoverflow.com/a/56766138/17575567)
template <typename T>
constexpr auto type_name_n() { //: Include namespaces
    std::string_view name, prefix, suffix;
    #ifdef __clang__
    name = __PRETTY_FUNCTION__;
    prefix = "auto type_name_n() [T = ";
    suffix = "]";
    #elif defined(__GNUC__)
    name = __PRETTY_FUNCTION__;
    prefix = "constexpr auto type_name_n() [with T = ";
    suffix = "]";
    #elif defined(_MSC_VER)
    name = __FUNCSIG__;
    prefix = "auto __cdecl type_name_n<";
    suffix = ">(void)";
    #endif
    name.remove_prefix(prefix.size());
    name.remove_suffix(suffix.size());
    return name;
}

template <typename T>
constexpr auto type_name() { //: Remove fresa namespaces, keep std::, glm::, ...
    std::string_view name = type_name_n<T>();
    if (name.find("Fresa::") != std::string_view::npos) {
        auto pos = name.find_last_of(':');
        if (pos != std::string_view::npos)
            name.remove_prefix(pos+1);
    }
    return name;
}

//: Gui
#ifndef DISABLE_GUI
#define IF_GUI(x) x
#else
#define IF_GUI(x)
#endif
