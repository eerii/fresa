//: fresa by jose pazos perez, licensed under GPLv3
#pragma once

//---------------------------------------------------
//  Core imports
//      - SDL2
//      - std vector, array, map
//      - std functional, memory
//---------------------------------------------------
#include <SDL2/SDL.h>

#include <vector>
#include <array>
#include <map>

#include <functional>
#include <memory>

//---------------------------------------------------
//  Custom types
//      - Aliases for unsigned integers
//      - Vec2 and Rect2 classes
//      - String helper (str alias as well as common manipulation functions)
//      - Variant helper (useful functions for working with variants)
//---------------------------------------------------
using ui8 = std::uint8_t;
using ui16 = std::uint16_t;
using ui32 = std::uint32_t;
using ui64 = std::uint64_t;

#include "vec2.h"
#include "rect2.h"

#include "string_helper.h"
#include "variant_helper.h"

//---------------------------------------------------
//  Compile time type checks for common types
//---------------------------------------------------
template<typename T> struct is_vector : std::false_type {};
template<typename A> struct is_vector<std::vector<A>> : std::true_type {};
template<typename T> inline constexpr bool is_vector_v=is_vector<T>::value;

template<class T> struct is_array : std::is_array<T>{};
template<class T, std::size_t N> struct is_array<std::array<T,N>> : std::true_type{};

template<typename T> struct is_vec2 : std::false_type {};
template<typename A> struct is_vec2<Fresa::Vec2<A>> : std::true_type {};
template<typename T> inline constexpr bool is_vec2_v=is_vec2<T>::value;

template<typename T> struct is_rect2 : std::false_type {};
template<typename A> struct is_rect2<Fresa::Rect2<A>> : std::true_type {};
template<typename T> inline constexpr bool is_rect2_v=is_rect2<T>::value;

//---------------------------------------------------
//  Compile time type name in readable format
//      https://stackoverflow.com/a/56766138/17575567
//      - type_name_n() includes the namespace
//      - type_name() removes the fresa namespaces (Fresa::, Fresa::Graphics::, ...) but keeps std::, glm::, ...
//---------------------------------------------------

/// Compile time type name (including all namespaces)
template <typename T>
constexpr auto type_name_n() {
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

/// Compile time type name
///
/// Includes common namespaces (std::, glm::) but excludes fresa namespaces (Fresa::*).
/// For example, the type name of `std::vector` is "std::vector", but the type name of `Fresa::Graphics::BufferData` is "BufferData".
template <typename T>
constexpr auto type_name() {
    std::string_view name = type_name_n<T>();
    if (name.find("Fresa::") != std::string_view::npos) {
        auto pos = name.find_last_of(':');
        if (pos != std::string_view::npos)
            name.remove_prefix(pos+1);
    }
    return name;
}

//---------------------------------------------------
//  Helper macros for specific code (Debug, GUI)
//---------------------------------------------------
#ifdef DEBUG
    #define IF_DEBUG(x) x
#else
    #define IF_DEBUG(x)
#endif

#ifndef DISABLE_GUI
    #define IF_GUI(x) x
#else
    #define IF_GUI(x)
#endif
