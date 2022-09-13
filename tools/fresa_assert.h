//* fresa_assert
//      custom runtime assertion, can be enabled/disabled using the FERESA_ASSERT macro
#pragma once

#include "fresa_config.h"
#include "source_loc.h"
#include "log.h"

namespace fresa
{
    template <typename ... T>
    inline void fresa_assert(bool condition, fmt::format_string<T...> fs, T&& ...t, const detail::source_location &location = detail::source_location::current()) {
        if constexpr (engine_config.enable_assertions()) {
            if (not condition) {
                str_view file_name = location.file_name();
                file_name = file_name.substr(file_name.find_last_of("/") + 1);
                detail::log<"ASSERTION ERROR", LogLevel::ERROR, fmt::color::red>("({}:{}) {}", file_name, location.line(), fmt::format(fs, std::forward<T>(t)...));
                std::abort();
            }
        }
    }
}