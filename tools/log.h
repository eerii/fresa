//* log
//      simple console logging system using the fmt library
//      includes different levels configurable by LOG_LEVEL
#pragma once

#include <iostream>
#include "fresa_types.h"
#include "fmt/core.h"

//* log levels
//      1 << 0 - errors
//      1 << 1 - warnings
//      1 << 2 - info
//      1 << 3 - graphics
//      1 << 4 - test errors
//      1 << 5 - test info and results
//      1 << 6 - debug

#ifndef LOG_LEVEL
#define LOG_LEVEL 0b0110111
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
        void test_error(const T& ...t) { detail::log<"TEST FAILED", 1 << 4>(t...); }

        template <typename ...T>
        void test_passed(const T& ...t) { detail::log<"TEST PASSED", 1 << 5>(t...); }

        template <typename ...T>
        void test(const T& ...t) { detail::log<"TEST", 1 << 5>(t...); }

        template <typename ...T>
        void debug(const T& ...t) { detail::log<"DEBUG", 1 << 6>(t...); }
    }
}