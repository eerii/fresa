//* log
//      simple console logging system using the fmt library
//      includes different levels configurable by LOG_LEVEL
#pragma once

#include "fresa_types.h"
#include "fmt/core.h"
#include "fmt/color.h"
#include "fmt/ranges.h"

namespace fresa
{
    //* log level
    enum log_levels {
        LOG_ERROR = 1 << 0,
        LOG_WARNING = 1 << 1,
        LOG_INFO = 1 << 2,
        LOG_GRAPHICS = 1 << 3,
        LOG_TEST = 1 << 4,
        LOG_DEBUG = 1 << 5,
        LOG_JOBS = 1 << 6,
    };

    #ifndef LOG_LEVEL
    #define LOG_LEVEL 0b0111111
    #endif

    namespace detail
    {
        template<str_literal name, std::size_t level, fmt::color color = fmt::color::white, typename ... T>
        constexpr void log(fmt::format_string<T...> fs, T&& ...t) {
            if constexpr((LOG_LEVEL & level) == level) {
                fmt::print("{} {}\n",
                           fmt::format(fg(color), "[{}]:", name.value),
                           fmt::format(fs, std::forward<T>(t)...));
            }
        }
    }

    namespace log
    {
        template <typename ... T>
        constexpr void error(fmt::format_string<T...> fs, T&& ...t) { detail::log<"ERROR", LOG_ERROR, fmt::color::red>(fs, std::forward<T>(t)...); }

        template <typename ... T>
        constexpr void warn(fmt::format_string<T...> fs, T&& ...t) { detail::log<"WARN", LOG_WARNING, fmt::color::gold>(fs, std::forward<T>(t)...); }

        template <typename ... T>
        constexpr void info(fmt::format_string<T...> fs, T&& ...t) { detail::log<"INFO", LOG_INFO, fmt::color::cornflower_blue>(fs, std::forward<T>(t)...); }

        template <typename ... T>
        constexpr void graphics(fmt::format_string<T...> fs, T&& ...t) { detail::log<"GRAPHICS", LOG_GRAPHICS, fmt::color::dark_turquoise>(fs, std::forward<T>(t)...); }

        template <typename ... T>
        constexpr void debug(fmt::format_string<T...> fs, T&& ...t) { detail::log<"DEBUG", LOG_DEBUG, fmt::color::light_sky_blue>(fs, std::forward<T>(t)...); }
    }
}