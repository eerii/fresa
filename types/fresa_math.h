//* fresa_math
//      linear algebra and other math utilities
#pragma once

#include "fresa_types.h"
#include <numbers>
#include <random>

namespace fresa
{
    //* mathematical constants
    constexpr auto pi = std::numbers::pi_v<float>;

    namespace concepts
    {
        //* concept that defines a number
        template <typename T>
        concept Numeric = std::integral<T> or std::floating_point<T>;

        //* concept that defines a matrix
        template <typename M>
        concept Matrix = requires(M m) {
            typename M::value_type; //: type of the matrix
            { std::remove_reference_t<decltype(M::size().first)>() } -> std::integral; //: number of rows
            { std::remove_reference_t<decltype(M::size().second)>() } -> std::integral; //: number of columns
            { std::remove_reference_t<decltype(m.template get<0, 0>())>() } -> Numeric; //: reference get
        } and requires(const M m) {
            { m.template get<0, 0>() } -> Numeric; //: const get
        };

        //* concept that defines a vector
        template <typename V>
        concept Vector = Matrix<V> and V::size().second == 1; //: a vector is a one dimensional matrix
    }

    //---
    //* linear algebra utilities

    //* scalar operations on vectors/matrices
    namespace detail
    {
        //: scalar operation
        template <concepts::Matrix M, typename Op>
        constexpr M scalar_op(const M& a, const typename M::value_type& b, Op op) {
            M result;
            for_<0, M::size().first>([&](auto i) {
                for_<0, M::size().second>([&](auto j) {
                    result.template get<i, j>() = op(a.template get<i, j>(), b);
                });
            });
            return result;
        }
    }
    //: scalar product
    template <concepts::Matrix M> constexpr M operator* (const M& a, const typename M::value_type& b) { return detail::scalar_op(a, b, std::multiplies()); }
    template <concepts::Matrix M> constexpr M operator* (const typename M::value_type& b, const M& a) { return a * b; }
    template <concepts::Matrix M> constexpr M& operator*= (M& a, const typename M::value_type& b) { return a = a * b; }
    //: scalar quotient
    template <concepts::Matrix M> constexpr M operator/ (const M& a, const typename M::value_type& b) { return detail::scalar_op(a, b, std::divides()); }
    template <concepts::Matrix M> constexpr M& operator/= (M& a, const typename M::value_type& b) { return a = a / b; }

    //* arithmetic operations on vectors/matrices
    namespace detail
    {
        //: binary operation
        template <concepts::Matrix M, typename Op>
        constexpr M binary_op(const M& a, const M& b, Op op) {
            M result;
            for_<0, M::size().first>([&](auto i) {
                for_<0, M::size().second>([&](auto j) {
                    result.template get<i, j>() = op(a.template get<i, j>(), b.template get<i, j>());
                });
            });
            return result;
        }
    }
    //: sum
    template <concepts::Matrix M> constexpr M operator+ (const M& a, const M& b) {  return detail::binary_op(a, b, std::plus()); }
    template <concepts::Matrix M> constexpr M& operator+= (M& a, const M& b) { return a = a + b; }
    //: difference
    template <concepts::Matrix M> constexpr M operator- (const M& a, const M& b) { return detail::binary_op(a, b, std::minus()); }
    template <concepts::Matrix M> constexpr M& operator-= (M& a, const M& b) { return a = a - b; }

    //* comparison operations on vectors/matrices
    namespace detail
    {
        //: comparison operation
        template <concepts::Matrix M, typename Op>
        constexpr bool compare_op(const M& a, const M& b, Op op) {
            bool result = true;
            for_<0, M::size().first>([&](auto i) {
                for_<0, M::size().second>([&](auto j) {
                    result = result and op(a.template get<i, j>(), b.template get<i, j>());
                });
            });
            return result;
        }
    }
    //: equality
    template <concepts::Matrix M> constexpr bool operator== (const M& a, const M& b) { return detail::compare_op(a, b, std::equal_to()); }
    //: inequality
    template <concepts::Matrix M> constexpr bool operator!= (const M& a, const M& b) { return not (a == b); }

    //* vector operations

    //: dot product
    template <concepts::Vector V> constexpr auto dot(const V& a, const V& b) {
        typename V::value_type result{};
        for_<0, V::size().first>([&](auto i) {
            for_<0, V::size().second>([&](auto j) {
                 result += a.template get<i, j>() * b.template get<i, j>();
            });
        });
        return result;
    }
    template <concepts::Vector V> constexpr auto operator* (const V& a, const V& b) { return dot(a, b); }

    //: cross product (3D)
    template <concepts::Vector V> requires (V::size().first == 3)
    constexpr V cross(const V& a, const V& b) {
        return V{a.template get<1, 0>() * b.template get<2, 0>() - a.template get<2, 0>() * b.template get<1, 0>(),
                 a.template get<2, 0>() * b.template get<0, 0>() - a.template get<0, 0>() * b.template get<2, 0>(),
                 a.template get<0, 0>() * b.template get<1, 0>() - a.template get<1, 0>() * b.template get<0, 0>()};
    }

    //: norm
    template <concepts::Vector V> constexpr auto norm(const V& a) { return std::sqrt(dot(a, a)); }

    //: normalize
    template <concepts::Vector V> requires std::floating_point<typename V::value_type> constexpr V normalize(const V& a) { return a / norm(a); }

    //: angle between vectors
    template <concepts::Vector V> constexpr auto angle(const V& a, const V& b) { return std::acos(dot(a, b) / (norm(a) * norm(b))); }

    //: angle with respect to the coordinate axis
    template <concepts::Vector V> constexpr auto angle_x(const V& a) { V vx{}; vx.template get<0, 0>() = 1; return angle(a, vx); }
    template <concepts::Vector V> requires (V::size().first >= 2) constexpr auto angle_y(const V& a) { V vy{}; vy.template get<1, 0>() = 1; return angle(a, vy); }
    template <concepts::Vector V> requires (V::size().first >= 3) constexpr auto angle_z(const V& a) { V vz{}; vz.template get<2, 0>() = 1; return angle(a, vz); }

    //---

    //* 2D vector
    template <concepts::Numeric T>
    struct Vec2 {
        using value_type = T;

        //: members
        T x, y;

        //: constructors
        Vec2() : x(0), y(0) {}
        Vec2(T x, T y) : x(x), y(y) {}

        //: size
        constexpr static std::pair<std::size_t, std::size_t> size() { return {2, 1}; }

        //: reference get
        template <std::size_t I, std::size_t J> requires (I < size().first and J == 0)
        constexpr T& get() {
            if constexpr (I == 0) return x;
            if constexpr (I == 1) return y;
        }

        //: const get
        template <std::size_t I, std::size_t J> requires (I < size().first and J == 0)
        constexpr T get() const {
            if constexpr (I == 0) return x;
            if constexpr (I == 1) return y;
        }
    };

    //: 3D vector
    template <concepts::Numeric T>
    struct Vec3 {
        using value_type = T;

        //: members
        T x, y, z;

        //: constructors
        Vec3() : x(0), y(0), z(0) {}
        Vec3(T x, T y, T z) : x(x), y(y), z(z) {}

        //: vector concept requirements
        constexpr static std::pair<std::size_t, std::size_t> size() { return {3, 1}; }

        //: reference get
        template <std::size_t I, std::size_t J> requires (I < size().first and J == 0)
        constexpr T& get() {
            if constexpr (I == 0) return x;
            if constexpr (I == 1) return y;
            if constexpr (I == 2) return z;
        }

        //: const get
        template <std::size_t I, std::size_t J> requires (I < size().first and J == 0)
        constexpr T get() const {
            if constexpr (I == 0) return x;
            if constexpr (I == 1) return y;
            if constexpr (I == 2) return z;
        }
    };

    //: Matrices
    template <std::size_t N, std::size_t M, concepts::Numeric T>
    struct Mat {
        using value_type = T;

        //: members
        std::array<T, N*M> data;

        //: constructors
        Mat() : data{} {}
        Mat(std::array<T, N*M>&& d) : data(std::move(d)) {}

        //: size
        constexpr static std::pair<std::size_t, std::size_t> size() { return {N, M}; }

        //: reference get
        template <std::size_t I, std::size_t J> requires (I < N and J < M)
        constexpr T& get() { return data[I * N + J]; }

        //: const get
        template <std::size_t I, std::size_t J> requires (I < N and J < M)
        constexpr T get() const { return data[I * N + J]; }
    };
    template <concepts::Numeric T>
    using Mat2 = Mat<2, 2, T>;
    template <concepts::Numeric T>
    using Mat3 = Mat<3, 3, T>;
    template <concepts::Numeric T>
    using Mat4 = Mat<4, 4, T>;

    //---

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