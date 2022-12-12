//* serialization
//     using reflection, we can serialize from/to text certain datatypes
#pragma once

#include "fresa_math.h"
#include "fresa_assert.h"
#include "string_utils.h"

#include <charconv>

namespace fresa
{
    // Base clase for serializer
    template <typename T>
    struct Serialize {
        [[nodiscard]] constexpr static T from(str_view s);
    };

    // Basic types
    template <std::integral T>
    struct Serialize<T> {
        [[nodiscard]] constexpr static T from(str_view s) {
            s = trim(s);
            T t;
            auto [ptr, error] = std::from_chars(s.data(), s.data() + s.size(), t);
            strong_assert(error == std::errc(), "Serialization error parsing integer"); 
            return t;
        }
    };

    template <std::floating_point T>
    struct Serialize<T> {
        [[nodiscard]] constexpr static T from(str_view s) {
            s = trim(s);
            T t;
            if constexpr (sizeof(T) == 4) {
                t = std::stof(str(s));
            } else if constexpr (sizeof(T) == 8) {
                t = std::stod(str(s));
            } else if constexpr (sizeof(T) == 16) {
                t = std::stold(str(s));
            } else {
                strong_assert(false, "Unsupported floating point type");
            }
            return t;
        }
    };

    template <>
    struct Serialize<bool> {
        [[nodiscard]] constexpr static bool from (str_view s) {
            s = trim(s);
            if (s == "true" or s == "1") return true;
            if (s == "false" or s == "0") return false;
            strong_assert(false, "Serialization error parsing boolean");
            return false;
        }
    };

    template <>
    struct Serialize<str> {
        [[nodiscard]] constexpr static str from(str_view s) {
            return str(trim(s));
        }
    };

    template <>
    struct Serialize<str_view> {
        [[nodiscard]] constexpr static str_view from(str_view s) {
            return trim(s);
        }
    };

    // Vector types
    template <typename T>
    struct Serialize<std::vector<T>> {
        [[nodiscard]] constexpr static std::vector<T> from(str_view s) {
            std::vector<T> v;
            for (const auto& token : split(s, ','))
                v.push_back(Serialize<T>::from(token));
            return v;
        }
    };

    template <typename T, size_t N>
    struct Serialize<std::array<T, N>> {
        [[nodiscard]] constexpr static std::array<T, N> from(str_view s) {
            auto v = Serialize<std::vector<T>>::from(s);
            strong_assert(v.size() == N, "Serialization error parsing array");
            std::array<T, N> a;
            std::copy_n(v.begin(), N, a.begin());
            return a;
        }
    };

    // Map types
    template <typename T, typename U>
    struct Serialize<std::unordered_map<T, U>> {
        [[nodiscard]] constexpr static std::unordered_map<T, U> from(str_view s) {
            std::unordered_map<T, U> m;
            str_view key_delim = ":";
            str_view item_delim = ",";
            for (const auto& token : split(s, ',')) {
                auto key_pos = token.find(key_delim);
                strong_assert(key_pos != str_view::npos, "Serialization error parsing map");
                auto key = Serialize<T>::from(token.substr(0, key_pos));
                auto value = Serialize<U>::from(token.substr(key_pos + key_delim.length()));
                m[key] = value;
            }
            return m;
        }
    };
}