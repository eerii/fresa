//* log
//      simple console logging system using the fmt library
//      includes different levels configurable by LOG_LEVEL
#pragma once

#include "fresa_config.h"
#include "fresa_enum.h"
#include "string_utils.h"

#include <type_traits>

#if __has_include("fmt/core.h")
    #include "fmt/core.h"
    #include "fmt/color.h"
    #include "fmt/ranges.h"
#else
    #error "fmt library not found"
#endif

namespace fresa
{
    //* log level
    enum struct LogLevel {
        ERROR = 1 << 0,
        WARNING = 1 << 1,
        INFO = 1 << 2,
        GRAPHICS = 1 << 3,
        TEST = 1 << 4,
        DEBUG = 1 << 5,
        JOBS = 1 << 6,
    };

    //* logging funcition, passes arguments to fmt::print to log it to the console
    namespace detail
    {
        template<str_literal name, LogLevel level, fmt::color color = fmt::color::white, typename ... T>
        constexpr void log(fmt::format_string<T...> fs, T&& ...t) {
            if constexpr(engine_config.log_level() & +level) {
                fmt::print("{} {}\n",
                           fmt::format(fg(color), "[{}]:", name.value),
                           fmt::format(fs, std::forward<T>(t)...));
            }
        }
    }

    //* specializations of log for different logging levels
    namespace log
    {
        template <typename ... T>
        constexpr void error(fmt::format_string<T...> fs, T&& ...t) { detail::log<"ERROR", LogLevel::ERROR, fmt::color::red>(fs, std::forward<T>(t)...); }

        template <typename ... T>
        constexpr void warn(fmt::format_string<T...> fs, T&& ...t) { detail::log<"WARN", LogLevel::WARNING, fmt::color::gold>(fs, std::forward<T>(t)...); }

        template <typename ... T>
        constexpr void info(fmt::format_string<T...> fs, T&& ...t) { detail::log<"INFO", LogLevel::INFO, fmt::color::cornflower_blue>(fs, std::forward<T>(t)...); }

        template <typename ... T>
        constexpr void graphics(fmt::format_string<T...> fs, T&& ...t) { detail::log<"GRAPHICS", LogLevel::GRAPHICS, fmt::color::dark_turquoise>(fs, std::forward<T>(t)...); }

        template <typename ... T>
        constexpr void debug(fmt::format_string<T...> fs, T&& ...t) { detail::log<"DEBUG", LogLevel::DEBUG, fmt::color::light_sky_blue>(fs, std::forward<T>(t)...); }
    }
}