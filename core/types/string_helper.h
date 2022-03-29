//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#include <string>
#include <array>
#include <vector>
#include <algorithm>
#include <ostream>

//: String
using str = std::string;

//: String view literals
using std::literals::string_view_literals::operator""sv;

namespace Fresa
{
    //: To lowercase
    inline str lower(str s) {
        for (auto &c : s)
            c = std::tolower(c);
        return s;
    }
    
    //: Trim
    static inline void trim_left(str &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {return !std::isspace(ch);}));
    }
    static inline void trim_right(str &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {return !std::isspace(ch);}).base(), s.end());
    }
    static inline void trim(str &s) {
        trim_left(s);
        trim_right(s);
    }
    
    //: Split
    //      - Recursive (deleting all consecutive delimiters, "a b" is the same as "a   b")
    //      - With lists (conserving [ ... ] in a single entry)
    inline std::vector<str> split(str s, str del = " ", bool recursive = false, bool lists = false) {
        std::vector<str> ss{};
        int a = 0;
        int b = (int)s.find(del);
        while (b != -1) {
            ss.push_back(s.substr(a, b - a));
            a = (int)(b + del.size());
            if (recursive) {
                while (s.size() > a and s.substr(a, del.size()) == del)
                    a++;
            }
            if (lists and s.find("[", a) == a) {
                b = (int)s.find("]", a) + 1;
                int c = (int)s.find("[", a+1) + 1; c = c == 0 ? (int)s.size() : c;
                if (b == 0 or b > c) throw std::runtime_error("[ERROR] The list is not closed, check the brackets");
                if (b == s.size()) b = -1;
            } else {
                b = (int)s.find(del, a);
            }
        }
        ss.push_back(s.substr(a, b - a));
        return ss;
    }
    
    //: Get list contents (from a string like '[a b c]' returns 'a b c')
    inline str list_contents(str s) {
        return s.substr(1, s.size() - 2);
    }
    
    //---Static string generation for compile time state machine tables and serialization---
    
    //: Array manipulation
    template <typename T, std::size_t N, std::size_t... I>
    constexpr std::array<T, N> toArray(T (&a)[N], std::index_sequence<I...>) {
        return {a[I]...};
    }
    template <typename T, std::size_t N>
    constexpr std::array<T, N> toArray(T (&a)[N]) {
        return toArray(a, std::make_index_sequence<N>());
    }

    template <typename T, std::size_t LSize, std::size_t RSize, std::size_t... LI, std::size_t... RI>
    constexpr std::array<T, LSize + RSize> joinArrays(const std::array<T, LSize>& la, const std::array<T, RSize>& ra,
                                                      std::index_sequence<LI...>, std::index_sequence<RI...>) {
        return { la[LI]..., ra[RI]... };
    }
    template <typename T, std::size_t LSize, std::size_t RSize>
    constexpr std::array<T, LSize + RSize> joinArrays(const std::array<T, LSize>& la, const std::array<T, RSize>& ra) {
        return joinArrays(la, ra, std::make_index_sequence<LSize>(), std::make_index_sequence<RSize>());
    }

    template <std::size_t NewSize, typename T, std::size_t PrevSize, std::size_t... I>
    constexpr std::array<T, NewSize> resizeArray(const std::array<T, PrevSize>& a, std::index_sequence<I...>) {
        return { a[I]... };
    }

    template <std::size_t NewSize, typename T, std::size_t PrevSize>
    constexpr std::array<T, NewSize> resizeArray(const std::array<T, PrevSize>& a) {
        constexpr std::size_t size = std::min(PrevSize, NewSize);
        return resizeArray<NewSize>(a, std::make_index_sequence<size>());
    }

    template <std::size_t NewSize, typename T, std::size_t PrevSize, std::size_t... I>
    constexpr std::array<T, NewSize> resizeArray(const std::array<T, PrevSize>& a, std::remove_const_t<T> fill, std::index_sequence<I...>) {
        return { ((I < PrevSize) ? a[I] : fill)... };
    }

    template <std::size_t NewSize, typename T, std::size_t PrevSize>
    constexpr std::array<T, NewSize> resizeArray(const std::array<T, PrevSize>& a, std::remove_const_t<T> fill) {
        return resizeArray<NewSize>(a, fill, std::make_index_sequence<NewSize>());
    }

    template <typename T, std::size_t N, std::size_t... I>
    constexpr bool areArraysEqual(const std::array<T, N>& la, const std::array<T, N>& ra, std::index_sequence<I...>) {
        return ((la[I] == ra[I]) && ...);
    }

    template <typename T, std::size_t N>
    constexpr bool areArraysEqual(const std::array<T, N>& la, const std::array<T, N>& ra) {
        return areArraysEqual(la, ra, std::make_index_sequence<N>());
    }

    //: Static string
    template <size_t N>
    struct Str {
        constexpr Str(const char (&p_ch)[N]): ch(toArray(p_ch)) {};
        constexpr Str(std::array<const char, N> p_ch): ch(std::move(p_ch)) {};
        std::array<const char, N> ch;
        
        template <std::size_t M>
        constexpr Str<N + M - 1> operator+(const Str<M> &rs) const {
            return joinArrays(resizeArray<N - 1>(ch), rs.ch);
        }
        
        constexpr bool operator==(const Str<N> &rs) const {
            return areArraysEqual(ch, rs.ch);
        }
        
        template <std::size_t L>
        constexpr Str<L + 1> changeLength(char fill) const
        {
            constexpr std::array<const char, 1> str_end{'\0'};
            return joinArrays(resizeArray<L>(resizeArray<N - 1>(ch, fill), fill), str_end);
        }
        
        constexpr std::size_t length() const {
            return N - 1;
        }
        
        constexpr const char* c_str() const {
            return ch.data();
        }
        
        constexpr std::string_view sv() const {
            return std::string_view(ch.data());
        }
        
        template <std::size_t M>
        friend struct Str;
    };
}
