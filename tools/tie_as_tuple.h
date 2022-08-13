//* tie_as_tuple
//      ***THIS IS AN AUTOGENERATED FILE***
//      do not modify, if you need more fields, use tie_as_tuple_generator.py
//      tie as tuple is a helper file for reflection, that decomposes any struct into its fields
//      unfortunately, since c++ does not support varadic parameter packs, we have to do the recursion manually
#pragma once

#include "struct_fields.h"
#include <tuple>

namespace fresa
{
    template <concepts::Aggregate T>
    constexpr auto tie_as_tuple(T& data) {
        constexpr auto fields = field_count_v<T>;
        static_assert(fields <= 32, "max supported members is 32");

        if constexpr (fields == 0) {
            return std::tie();
        } else if constexpr (fields == 1) {
            auto& [a1] = data;
            return std::tie(a1);
        } else if constexpr (fields == 2) {
            auto& [a1, a2] = data;
            return std::tie(a1, a2);
        } else if constexpr (fields == 3) {
            auto& [a1, a2, a3] = data;
            return std::tie(a1, a2, a3);
        } else if constexpr (fields == 4) {
            auto& [a1, a2, a3, a4] = data;
            return std::tie(a1, a2, a3, a4);
        } else if constexpr (fields == 5) {
            auto& [a1, a2, a3, a4, a5] = data;
            return std::tie(a1, a2, a3, a4, a5);
        } else if constexpr (fields == 6) {
            auto& [a1, a2, a3, a4, a5, a6] = data;
            return std::tie(a1, a2, a3, a4, a5, a6);
        } else if constexpr (fields == 7) {
            auto& [a1, a2, a3, a4, a5, a6, a7] = data;
            return std::tie(a1, a2, a3, a4, a5, a6, a7);
        } else if constexpr (fields == 8) {
            auto& [a1, a2, a3, a4, a5, a6, a7, a8] = data;
            return std::tie(a1, a2, a3, a4, a5, a6, a7, a8);
        } else if constexpr (fields == 9) {
            auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9] = data;
            return std::tie(a1, a2, a3, a4, a5, a6, a7, a8, a9);
        } else if constexpr (fields == 10) {
            auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10] = data;
            return std::tie(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
        } else if constexpr (fields == 11) {
            auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11] = data;
            return std::tie(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
        } else if constexpr (fields == 12) {
            auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12] = data;
            return std::tie(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
        } else if constexpr (fields == 13) {
            auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13] = data;
            return std::tie(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
        } else if constexpr (fields == 14) {
            auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14] = data;
            return std::tie(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
        } else if constexpr (fields == 15) {
            auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15] = data;
            return std::tie(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
        } else if constexpr (fields == 16) {
            auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16] = data;
            return std::tie(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
        } else if constexpr (fields == 17) {
            auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17] = data;
            return std::tie(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17);
        } else if constexpr (fields == 18) {
            auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18] = data;
            return std::tie(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18);
        } else if constexpr (fields == 19) {
            auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19] = data;
            return std::tie(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19);
        } else if constexpr (fields == 20) {
            auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20] = data;
            return std::tie(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
        } else if constexpr (fields == 21) {
            auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21] = data;
            return std::tie(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21);
        } else if constexpr (fields == 22) {
            auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22] = data;
            return std::tie(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22);
        } else if constexpr (fields == 23) {
            auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23] = data;
            return std::tie(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23);
        } else if constexpr (fields == 24) {
            auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24] = data;
            return std::tie(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24);
        } else if constexpr (fields == 25) {
            auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25] = data;
            return std::tie(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25);
        } else if constexpr (fields == 26) {
            auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26] = data;
            return std::tie(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26);
        } else if constexpr (fields == 27) {
            auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27] = data;
            return std::tie(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27);
        } else if constexpr (fields == 28) {
            auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28] = data;
            return std::tie(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28);
        } else if constexpr (fields == 29) {
            auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29] = data;
            return std::tie(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29);
        } else if constexpr (fields == 30) {
            auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30] = data;
            return std::tie(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30);
        } else if constexpr (fields == 31) {
            auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31] = data;
            return std::tie(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31);
        } else if constexpr (fields == 32) {
            auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32] = data;
            return std::tie(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32);
        } 
    }
}