//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

//---Static string generation for compile time state machine tables---

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

namespace Testing {

    namespace {
    
    [[maybe_unused]] constexpr void testToStdArray()
    {
        constexpr int input[] = {1, 2, 3};
        constexpr auto output = Fresa::toArray(input);
        constexpr std::array<const int, 3> expected = {1, 2, 3};
        static_assert(Fresa::areArraysEqual(expected, output));
    }

    [[maybe_unused]] constexpr void testJoin()
    {
        constexpr std::array inputA = {1, 2, 3};
        constexpr std::array inputB = {4, 5};
        constexpr std::array expected = {1, 2, 3, 4, 5};
        static_assert(Fresa::areArraysEqual(expected, Fresa::joinArrays(inputA, inputB)));
    }

    [[maybe_unused]] constexpr void testResize()
    {
        constexpr std::array input = {1, 2, 3};
        constexpr std::array expectedShorter = {1, 2};
        constexpr std::array expectedLonger = {1, 2, 3, 0};
        static_assert(Fresa::areArraysEqual(expectedShorter, Fresa::resizeArray<2>(input)));
        static_assert(Fresa::areArraysEqual(expectedLonger, Fresa::resizeArray<4>(input)));
    }
    
    [[maybe_unused]] constexpr void testAdding()
    {
        constexpr Fresa::Str ls{"abc"};
        constexpr Fresa::Str rs{"de"};
        constexpr Fresa::Str expected{"abcde"};
        static_assert(expected == ls + rs);
    }
    
    [[maybe_unused]] constexpr void testLength()
    {
        constexpr Fresa::Str ls{"abc"};
        constexpr size_t expected{3};
        static_assert(ls.length() == expected);
    }

    [[maybe_unused]] constexpr void test0Length()
    {
        constexpr Fresa::Str ls{""};
        constexpr size_t expected{0};
        static_assert(ls.length() == expected);
    }

    [[maybe_unused]] constexpr void testChangeLength()
    {
        constexpr Fresa::Str shorter{"abc"};
        constexpr Fresa::Str longer{"abcdef"};
        constexpr Fresa::Str empty{""};

        constexpr size_t l{5};
        constexpr Fresa::Str expectedShorter{"abcxx"};
        constexpr Fresa::Str expectedLonger {"abcde"};
        constexpr Fresa::Str expectedEmpty  {"zzzzz"};

        constexpr auto res = shorter.changeLength<l>('x');

        static_assert(res.c_str()[3] == expectedShorter.c_str()[3]);

        static_assert(shorter.changeLength<l>('x') == expectedShorter);
        static_assert(longer.changeLength<l>('y') == expectedLonger);
        static_assert(empty.changeLength<l>('z') == expectedEmpty);
    }
    
    }
}
