//* fresa_math
//      linear algebra and other math utilities
#pragma once

#include "std_types.h"
#include <random>

namespace fresa
{
    namespace concepts
    {
        //* concept that defines a number
        template <typename T>
        concept Numeric = std::integral<T> or std::floating_point<T>;
    }

    //* random number generator
    //      returns a random number between min and max using the mt19937 generator and an uniform distribution
    //      the interval is clossed [min, max], so both are included
    template <concepts::Numeric T = int>
    T random(T min, T max) {
        static std::random_device r;

        using NumberGenerator = decltype([]{
            if constexpr (sizeof(T) > 4)
                return std::mt19937_64();
            else
                return std::mt19937();
        }());
        static NumberGenerator rng(r());

        using Distribution = decltype([]{
            if constexpr (std::integral<T>)
                return std::uniform_int_distribution<T>();
            else
                return std::uniform_real_distribution<T>();
        }());
        Distribution dist(min, max);

        return dist(rng);
    }
}