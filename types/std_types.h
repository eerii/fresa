//* std_types
//      includes common standard types that are used engine-wide
//      also creates aliases for ease of use of common types
#pragma once

//* unsigned type aliases
#include <cstdint>
namespace fresa
{
    using ui8 = std::uint8_t;
    using ui16 = std::uint16_t;
    using ui32 = std::uint32_t;
    using ui64 = std::uint64_t;
}

//* string type aliases
#include <string>
#include <string_view>
namespace fresa
{
    using str = std::string;
    using str_view = std::string_view;
}

//* containers
#include <array>
#include <vector>
#include <map>
#include <unordered_map>

//* concepts
#ifdef __cpp_lib_concepts
    #include <concepts>
#else
    #error "concepts not supported"
#endif

//* ranges
#if __has_include(<range/v3/all.hpp>)
    #include <range/v3/all.hpp>
    namespace fresa
    {
        namespace rv = ranges::views;
    }
#elif defined __cpp_lib_ranges
    #include <ranges>
    namespace fresa
    {
        namespace rv = std::ranges::views;
        namespace ranges = std::ranges;
    }
#else
    #error "ranges not supported"
#endif

//* coroutines
#if __has_include(<coroutine>)
    #include <coroutine>
    namespace std_ = std;
#elif __has_include(<experimental/coroutine>)
    #include <experimental/coroutine>
    namespace std_ = std::experimental;
#else
    #error "coroutines not supported"
#endif

//* new jthread
#include <thread>
#if defined __cpp_lib_jthread
    //: jthread is supported
#elif __has_include("jthread.hpp") and __has_include("stop_token.hpp")
    #include "jthread.hpp"
    #include "stop_token.hpp"
#else
    #error "jthread not supported"
#endif