//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "arrays.h"

namespace Verse
{
    template <std::size_t N>
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
        
        constexpr const char* c_str() const {
            return ch.data();
        }
        
        template <std::size_t M>
        friend struct Str;
    };
}

namespace Test {

    namespace {
    
    [[maybe_unused]] constexpr void testAdding()
    {
        constexpr Verse::Str ls{"abc"};
        constexpr Verse::Str rs{"de"};
        constexpr Verse::Str expected{"abcde"};
        static_assert(expected == ls + rs);
    }
    
    }
}
