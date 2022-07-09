//* type_name
//      this header provides a type name function which creates a constexpr string view with the name of a type
//      it also includes functionality to create type hashes
#pragma once

#include "std_types.h"
#include "strong_types.h"

namespace fresa
{
    //* type name
    //      type_name_n returns the name of the type including all namespaces
    //      type_name returns the name of the type excluding fresa namespaces
    //      relevant discussion in https://stackoverflow.com/questions/81870/is-it-possible-to-print-a-variables-type-in-standard-c

    //: forward declaration of type_name_t to allow overloading
    template <typename T> constexpr str_view type_name_n();

    //: type_name_t is preinitialized for void, this way we can automatically
    //  calculate compiler specific prefixes and suffixes and trim the other names accordingly
    template <> [[nodiscard]] constexpr str_view type_name_n<void>() { return "void"; }

    namespace detail
    {
        //: compiler specific type representations
        template <typename T>
        constexpr str_view compiler_type_name() {
            #if defined __clang__
                return __PRETTY_FUNCTION__;
            #elif defined __GNUC__
                return __PRETTY_FUNCTION__;
            #elif defined _MSC_VER
                return __FUNCSIG__;
            #else
                #error "compiler not supported"
            #endif
        }

        //: calculate prefix and suffix length using the precalculated name for void
        using type_probe = void;
        constexpr std::size_t compiler_type_prefix() {
            return compiler_type_name<type_probe>().find(type_name_n<type_probe>());
        }
        constexpr std::size_t compiler_type_suffix() {
            return compiler_type_name<type_probe>().length() - compiler_type_prefix() - type_name_n<type_probe>().length();
        }
    }

    //: constexpr type names including all namespaces
    //  to exclude fresa namespaces, use type_name<T>()
    template <typename T>
    [[nodiscard]] constexpr str_view type_name_n() {
        constexpr auto compiler_name = detail::compiler_type_name<T>();
        return compiler_name.substr(detail::compiler_type_prefix(), compiler_name.length() - detail::compiler_type_prefix() - detail::compiler_type_suffix());
    }

    //: constexpr type names excluding fresa namespaces
    //  for example, "std::vector" will remain the same, but "fresa::graphics::Buffer" will return "Buffer"
    //  to include namespaces use, use type_name_n<T>()
    template <typename T>
    [[nodiscard]] constexpr str_view type_name() {
        constexpr auto name = type_name_n<T>();
        if (name.find("fresa::") != str_view::npos) {
            auto pos = name.find_last_of("::");
            return name.substr(pos + 1);
        }
        return name;
    }

    //* type hash
    //      hashes the type name to allow for type lookup in an unordered map

    constexpr ui64 hash_fnv1a(str_view s) {
        constexpr ui64 fnv_prime = 0x100000001b3;
        ui64 result = 0xcbf29ce484222325;
        for (auto c : s) {
            result ^= c;
            result *= fnv_prime;
        }
        return result;
    }

    using TypeHash = strong::Type<ui64, decltype([]{}), strong::Hashable>;

    template <typename T>
    [[nodiscard]] constexpr TypeHash type_hash() {
        constexpr auto name = type_name_n<T>();
        return hash_fnv1a(name);
    }
}