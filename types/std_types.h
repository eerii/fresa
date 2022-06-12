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
#include <vector>
//? think about which containers to use, vector is a must, but what about map and set?
//? functional, memory