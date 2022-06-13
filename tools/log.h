//* log
//      simple console logging system using the fmt library
//      includes different levels configurable by LOG_LEVEL
#pragma once

#include "fresa_types.h"
#include "fmt/core.h"

//* log levels
//      1 << 0 - errors
//      1 << 1 - warnings
//      1 << 2 - info
//      1 << 3 - graphics
//      1 << 4 - test
//      1 << 5 - debug

#ifndef LOG_LEVEL
#define LOG_LEVEL 0b010111
#endif

namespace fresa
{
    namespace detail
    {
        template<str_literal name, std::size_t level, typename ...T>
        void log(const T& ...t) {
            if constexpr(LOG_LEVEL & level) {
                fmt::print("[{}]: ", name.value);
                fmt::print(t...);
                fmt::print("\n");
            }
        }
    }

    namespace log
    {
        template <typename ...T>
        void error(const T& ...t) { detail::log<"ERROR", 1 << 0>(t...); }

        template <typename ...T>
        void warn(const T& ...t) { detail::log<"WARN", 1 << 1>(t...); }

        template <typename ...T>
        void info(const T& ...t) { detail::log<"INFO", 1 << 2>(t...); }

        template <typename ...T>
        void graphics(const T& ...t) { detail::log<"GRAPHICS", 1 << 3>(t...); }

        template <typename ...T>
        void test(const T& ...t) { detail::log<"TEST", 1 << 4>(t...); }

        template <typename ...T>
        void debug(const T& ...t) { detail::log<"DEBUG", 1 << 5>(t...); }
    }
}