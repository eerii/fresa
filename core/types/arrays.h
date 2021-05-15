//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "dtypes.h"

namespace Verse
{
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

    template <typename T, std::size_t N, std::size_t... I>
    constexpr bool areArraysEqual(const std::array<T, N>& la, const std::array<T, N>& ra, std::index_sequence<I...>) {
        return ((la[I] == ra[I]) && ...);
    }

    template <typename T, std::size_t N>
    constexpr bool areArraysEqual(const std::array<T, N>& la, const std::array<T, N>& ra) {
        return areArraysEqual(la, ra, std::make_index_sequence<N>());
    }
}

namespace Test {

    namespace {

    [[maybe_unused]] constexpr void testToStdArray()
    {
        constexpr int input[] = {1, 2, 3};
        constexpr auto output = Verse::toArray(input);
        constexpr std::array<const int, 3> expected = {1, 2, 3};
        static_assert(Verse::areArraysEqual(expected, output));
    }

    [[maybe_unused]] constexpr void testJoin()
    {
        constexpr std::array inputA = {1, 2, 3};
        constexpr std::array inputB = {4, 5};
        constexpr std::array expected = {1, 2, 3, 4, 5};
        static_assert(Verse::areArraysEqual(expected, Verse::joinArrays(inputA, inputB)));
    }

    [[maybe_unused]] constexpr void testResize()
    {
        constexpr std::array input = {1, 2, 3};
        constexpr std::array expectedShorter = {1, 2};
        constexpr std::array expectedLonger = {1, 2, 3, 0};
        static_assert(Verse::areArraysEqual(expectedShorter, Verse::resizeArray<2>(input)));
        static_assert(Verse::areArraysEqual(expectedLonger, Verse::resizeArray<4>(input)));
    }

    }

}
