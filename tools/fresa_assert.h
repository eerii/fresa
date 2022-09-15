//* fresa_assert
//      custom runtime assertion, soft asserts can be enabled/disabled using the enable_assertions engine configurations
#pragma once

#include "fresa_config.h"
#include "source_loc.h"
#include "log.h"
#include "engine.h"

namespace fresa
{
    //* soft assert
    //      general purpose debug only assertion, can be disabled with the enable_assertions config flag
    //      it is meant to make tests only during the development of the engine, they will not be present in production
    //      this will completely abort the program execution if the assertion fails
    template <typename ... T>
    inline void soft_assert(bool condition, fmt::format_string<T...> fs, T&& ...t, const detail::source_location &location = detail::source_location::current()) {
        if constexpr (engine_config.enable_assertions()) {
            if (not condition) {
                str_view file_name = location.file_name();
                file_name = file_name.substr(file_name.find_last_of("/") + 1);
                detail::log<"ASSERTION ERROR", LogLevel::ERROR, fmt::color::red>("({}:{}) {}", file_name, location.line(), fmt::format(fs, std::forward<T>(t)...));
                std::abort();
            }
        }
    }

    //* strong assert
    //      this assertion is always enabled, and it is meant to be used as an error handling mechanism instead of an if statement
    //      contrary to a soft assert, it will call force_quit instead of aborting the program
    template <typename ... T>
    void strong_assert(bool condition, fmt::format_string<T...> fs, T&& ...t, const detail::source_location &location = detail::source_location::current()) {
        if (not condition) {
            str_view file_name = location.file_name();
            file_name = file_name.substr(file_name.find_last_of("/") + 1);
            detail::log<"ASSERTION ERROR", LogLevel::ERROR, fmt::color::red>("({}:{}) {}", file_name, location.line(), fmt::format(fs, std::forward<T>(t)...));
            fresa::force_quit();
        }
    }
}