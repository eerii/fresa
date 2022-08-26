//* fresa_time
//      time management system, based on std::chrono
#pragma once

#include <chrono>



//* use literals for specifying time duration, for example, 10s or 3ms
using namespace std::literals::chrono_literals;

namespace fresa
{
    //* std::chrono clock
    using clock = std::chrono::steady_clock;

    //* get current time
    [[nodiscard]] inline clock::time_point time() { return clock::now(); }

    //* fixed delta time for updates, new simulation each 1/N seconds
    auto constexpr update_frequency = 100;
    auto constexpr dt = std::chrono::duration<ui64, std::ratio<1, update_frequency>>(1);
}

//* custom time difference formatter for the fmt library
 #if __has_include("fmt/format.h")
    #include "fmt/format.h"
    namespace fresa::detail
    {
        using time_difference = decltype(time() - time());
    }
    template <>
    struct fmt::formatter<fresa::detail::time_difference> : fmt::formatter<float> {
        template <typename FormatContext> constexpr auto format(const fresa::detail::time_difference& d, FormatContext& ctx) noexcept {
            auto value = d.count();
            auto units = [&]() -> std::pair<float, fresa::str_view> {
                if (value < 1e3) return {1e-0, "ns"};
                if (value < 1e6) return {1e-3, "us"};
                if (value < 1e9) return {1e-6, "ms"};
                return {1e-9, "s"};
            }();
            fmt::formatter<float>::format(value * units.first, ctx);
            return fmt::format_to(ctx.out(), "{}", units.second);
        }
    };
#endif