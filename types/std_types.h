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
//-  create custom string id with hash, implement in another file

//* containers
#include <array>
#include <vector>
#include <map>
#include <unordered_map>
//- directed graph implementation
//- binary tree implementation

//* concepts
#ifdef __cpp_lib_concepts
    #include <concepts>
#else
    #error "concepts not supported"
#endif

//* ranges
#ifdef __cpp_lib_ranges
    #include <ranges>
    namespace fresa
    {
        namespace rv = std::ranges::views;
        namespace ranges = std::ranges;
    }
#elif __has_include(<range/v3/all.hpp>)
    #include <range/v3/all.hpp>
    namespace fresa
    {
        namespace rv = ranges::views;
    }
#else
    #error "ranges not supported"
#endif