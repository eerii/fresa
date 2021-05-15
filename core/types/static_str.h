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
    
    [[maybe_unused]] constexpr void testLength()
    {
        constexpr Verse::Str ls{"abc"};
        constexpr size_t expected{3};
        static_assert(ls.length() == expected);
    }

    [[maybe_unused]] constexpr void test0Length()
    {
        constexpr Verse::Str ls{""};
        constexpr size_t expected{0};
        static_assert(ls.length() == expected);
    }

    [[maybe_unused]] constexpr void testChangeLength()
    {
        constexpr Verse::Str shorter{"abc"};
        constexpr Verse::Str longer{"abcdef"};
        constexpr Verse::Str empty{""};

        constexpr size_t l{5};
        constexpr Verse::Str expectedShorter{"abcxx"};
        constexpr Verse::Str expectedLonger {"abcde"};
        constexpr Verse::Str expectedEmpty  {"zzzzz"};

        constexpr auto res = shorter.changeLength<l>('x');

        static_assert(res.c_str()[3] == expectedShorter.c_str()[3]);

        static_assert(shorter.changeLength<l>('x') == expectedShorter);
        static_assert(longer.changeLength<l>('y') == expectedLonger);
        static_assert(empty.changeLength<l>('z') == expectedEmpty);
    }
    
    }
}
