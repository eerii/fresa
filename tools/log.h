//* log
//      simple console logging system using the fmt library
//      includes different levels configurable by LOG_LEVEL
#pragma once

#include <iostream>
#include "fresa_types.h"
#include "fmt/core.h"
#include "fmt/color.h"

namespace fresa
{
    //* log level
    enum log_levels {
        LOG_ERROR = 1 << 0,
        LOG_WARNING = 1 << 1,
        LOG_INFO = 1 << 2,
        LOG_GRAPHICS = 1 << 3,
        LOG_TEST = 1 << 4,
        LOG_DEBUG = 1 << 5
    };

    #ifndef LOG_LEVEL
    #define LOG_LEVEL 0b111111
    #endif

    namespace detail
    {
        template<str_literal name, std::size_t level, fmt::color color = fmt::color::white, typename ...T>
        void log(const T& ...t) {
            if constexpr((LOG_LEVEL & level) == level) {
                fmt::print(fg(color), "[{}]: ", name.value);
                fmt::print(t...);
                fmt::print("\n");
            }
        }
    }

    namespace log
    {
        template <typename ...T>
        void error(const T& ...t) { detail::log<"ERROR", LOG_ERROR, fmt::color::red>(t...); }

        template <typename ...T>
        void warn(const T& ...t) { detail::log<"WARN", LOG_WARNING, fmt::color::gold>(t...); }

        template <typename ...T>
        void info(const T& ...t) { detail::log<"INFO", LOG_INFO, fmt::color::cornflower_blue>(t...); }

        template <typename ...T>
        void graphics(const T& ...t) { detail::log<"GRAPHICS", LOG_GRAPHICS, fmt::color::dark_turquoise>(t...); }

        template <typename ...T>
        void debug(const T& ...t) { detail::log<"DEBUG", LOG_DEBUG, fmt::color::light_sky_blue>(t...); }
    }
}