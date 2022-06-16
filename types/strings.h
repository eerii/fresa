//* strings
//      helper functions for strings and string views

#include "std_types.h"

namespace fresa
{
    //* string literals for templates
    template <auto N>
    struct str_literal {
        constexpr str_literal(const char (&s)[N]) { std::copy(s, s + N, value); }
        char value[N];
    };

    //* lowercase string view
    constexpr str_view lower(str_view s) {
        char* l = new char[s.size()]; // new c++20 dynamic allocation in constexpr
        std::transform(s.begin(), s.end(), l, [](char c) { return std::tolower(c); });
        s = str_view{l, s.size()};
        return s;
    }
    constexpr str_view operator""_lower(const char* text, std::size_t size) {
        return lower(str_view{text, size});
    }

    //* split
    constexpr auto split(str_view s, char del = ' ') {
        return s | rv::split(del)
                 | rv::transform([](auto &&r) { return str_view(&*r.begin(), ranges::distance(r)); })
                 | rv::filter([](auto &&s) { return !s.empty(); });
    }

    //- string hash
}