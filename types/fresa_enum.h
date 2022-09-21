//* fresa enum
//      some utilities for custom enum structures, like bitwise operators
#pragma once

#include "std_types.h"

namespace fresa
{
    //: detect if a type is an scoped enum
    //      core in c++23, implementation from https://en.cppreference.com/w/cpp/types/is_scoped_enum
    namespace concepts
    {
        namespace detail
        {
            template <typename T>
            concept is_sizable = requires { sizeof(T); };

            template <typename T>
            concept is_implicitly_convertible_to_int = requires (T t) { t + 1; };
        }

        template <typename E>
        concept Enum = std::is_enum_v<E>;

        template <typename E>
        concept ScopedEnum = Enum<E> and detail::is_sizable<E> and not detail::is_implicitly_convertible_to_int<E>;
    }

    //: cast to underlying type using + operator
    template <concepts::ScopedEnum E> constexpr auto operator+ (E e) {return static_cast<std::underlying_type_t<E>>(e);}

    //: bitwise operators with the same type
    template <concepts::ScopedEnum E> constexpr E operator| (E a, E b) {return static_cast<E>(+a | +b);}
    template <concepts::ScopedEnum E> constexpr E operator& (E a, E b) {return static_cast<E>(+a & +b);}

    //: bitwise assignment operators
    template <concepts::ScopedEnum E> constexpr E& operator|= (E& a, E b) {return a = a | b;}
    template <concepts::ScopedEnum E> constexpr E& operator&= (E& a, E b) {return a = a & b;}

    //: bitwise comparison operators
    template <concepts::ScopedEnum E> constexpr bool operator== (E a, E b) {return +a == +b;}

    //: bitwise operators with the underlying type
    template <concepts::ScopedEnum E> constexpr std::underlying_type_t<E> operator| (E a, std::underlying_type_t<E> b) {return +a | b;}
    template <concepts::ScopedEnum E> constexpr std::underlying_type_t<E> operator& (E a, std::underlying_type_t<E> b) {return +a & b;}
    template <concepts::ScopedEnum E> constexpr bool operator== (E a, std::underlying_type_t<E> b) {return +a == b;}
}