//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#include <array>
#include <string>

//---Static string generation for compile time state machine tables and serialization---

namespace Fresa
{
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
