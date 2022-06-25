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

        //* concept that defines a square matrix
        template <typename M>
        concept SquareMatrix = Matrix<M> and M::size().first == M::size().second;

        //* concept that defines a vector
        template <typename V>
        concept ColumnVector = Matrix<V> and V::size().second == 1 and V::size().first > 1;
        template <typename V>
        concept RowVector = Matrix<V> and V::size().first == 1 and V::size().second > 1;
        template <typename V>
        concept Vector = ColumnVector<V> or RowVector<V>;
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

    //---
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
    template <concepts::ColumnVector V> requires (V::size().first == 3)
    constexpr V cross(const V& a, const V& b) {
        return V{a.template get<1, 0>() * b.template get<2, 0>() - a.template get<2, 0>() * b.template get<1, 0>(),
                 a.template get<2, 0>() * b.template get<0, 0>() - a.template get<0, 0>() * b.template get<2, 0>(),
                 a.template get<0, 0>() * b.template get<1, 0>() - a.template get<1, 0>() * b.template get<0, 0>()};
    }

    //: norm
    template <concepts::Vector V> constexpr auto norm(const V& v) { return std::sqrt(dot(v, v)); }

    //: normalize
    template <concepts::Vector V> requires std::floating_point<typename V::value_type> constexpr V normalize(const V& v) { return v / norm(v); }

    //: angle between vectors
    template <concepts::Vector V> constexpr auto angle(const V& a, const V& b) { return std::acos(dot(a, b) / (norm(a) * norm(b))); }

    //: angle with respect to the coordinate axis
    template <concepts::ColumnVector V> constexpr auto angle_x(const V& v) { V vx{}; vx.template get<0, 0>() = 1; return angle(v, vx); }
    template <concepts::ColumnVector V> requires (V::size().first >= 2) constexpr auto angle_y(const V& v) { V vy{}; vy.template get<1, 0>() = 1; return angle(v, vy); }
    template <concepts::ColumnVector V> requires (V::size().first >= 3) constexpr auto angle_z(const V& v) { V vz{}; vz.template get<2, 0>() = 1; return angle(v, vz); }

    //---
    //* transformations

    //: to convertible type
    template <concepts::Matrix B, concepts::Matrix A> requires (A::size().first == A::size().first and A::size().second == B::size().second)
    constexpr B to(const A& a) {
        B result;
        for_<0, A::size().first>([&](auto i) {
            for_<0, A::size().second>([&](auto j) {
                result.template get<i, j>() = a.template get<i, j>();
            });
        });
        return result;
    }

    //: to column vector
    template <concepts::ColumnVector C, concepts::RowVector R> requires (C::size().first == R::size().second)
    constexpr C to_column(const R& v) {
        C result;
        for_<0, R::size().second>([&](auto i) {
            result.template get<i, 0>() = v.template get<0, i>();
        });
        return result;
    }

    //: to row vector
    template <concepts::RowVector R, concepts::ColumnVector C> requires (C::size().first == R::size().second)
    constexpr R to_row(const C& v) {
        R result;
        for_<0, C::size().first>([&](auto i) {
            result.template get<0, i>() = v.template get<i, 0>();
        });
        return result;
    }

    //: row vector by column vector multiplication (regular dot product)
    template <concepts::RowVector R, concepts::ColumnVector C>
    requires (C::size().first == R::size().second)
    constexpr auto dot(const R& r, const C& c) { auto r_ = to_column<C>(r); return dot(r_, c); }
    template <concepts::RowVector R, concepts::ColumnVector C>
    requires (C::size().first == R::size().second)
    constexpr auto operator* (const R& r, const C& c) { return dot(r, c); }

    //---
    //* matrix operations

    //: matrix multiplication
    template <concepts::Matrix M, concepts::Matrix A, concepts::Matrix B>
    requires (A::size().second == B::size().first and M::size().first == A::size().first and M::size().second == B::size().second)
    constexpr M dot(const A& a, const B& b) {
        M result;
        for_<0, A::size().first>([&](auto i) {
            for_<0, B::size().second>([&](auto j) {
                for_<0, A::size().second>([&](auto k) {
                    result.template get<i, j>() += a.template get<i, k>() * b.template get<k, j>();
                });
            });
        });
        return result;
    }

    //: multiplication for square matrices
    template <concepts::SquareMatrix M>
    constexpr M dot(const M& a, const M& b) { return dot<M, M, M>(a, b); }
    template <concepts::SquareMatrix M>
    constexpr M operator* (const M& a, const M& b) { return dot<M, M, M>(a, b); }

    //: transpose
    template <concepts::SquareMatrix M>
    constexpr M transpose(const M& a) {
        M result;
        for_<0, M::size().first>([&](auto i) {
            for_<0, M::size().second>([&](auto j) {
                result.template get<j, i>() = a.template get<i, j>();
            });
        });
        return result;
    }

    //: matrix vector multiplication
    template <concepts::SquareMatrix M, concepts::ColumnVector V> requires (M::size().second == V::size().first)
    constexpr V dot(const M& m, const V& v) { return dot<V, M, V>(m, v); }
    template <concepts::SquareMatrix M, concepts::ColumnVector V> requires (M::size().second == V::size().first)
    constexpr V operator* (const M& m, const V& v) { return dot<V, M, V>(m, v); }
    template <concepts::SquareMatrix M, concepts::RowVector V> requires (M::size().first == V::size().second)
    constexpr V dot(const V& v, const M& m) { return dot<V, V, M>(v, m); }
    template <concepts::SquareMatrix M, concepts::RowVector V> requires (M::size().first == V::size().second)
    constexpr V operator* (const V& v, const M& m) { return dot<V, V, M>(v, m); }

    //---

    //* 2D vector (column)
    template <concepts::Numeric T>
    struct Vec2 {
        using value_type = T;

        //: members
        T x, y;

        //: constructors
        constexpr Vec2() : x(0), y(0) {}
        constexpr Vec2(T x, T y) : x(x), y(y) {}

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

    //: 3D vector (column)
    template <concepts::Numeric T>
    struct Vec3 {
        using value_type = T;

        //: members
        T x, y, z;

        //: constructors
        constexpr Vec3() : x(0), y(0), z(0) {}
        constexpr Vec3(T x, T y, T z) : x(x), y(y), z(z) {}

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
        constexpr Mat() : data{} {}
        constexpr Mat(std::array<T, N*M>&& d) : data(std::move(d)) {}

        //: size
        constexpr static std::pair<std::size_t, std::size_t> size() { return {N, M}; }

        //: reference get
        template <std::size_t I, std::size_t J> requires (I < N and J < M)
        constexpr T& get() { return data.at(I * M + J); }

        //: const get
        template <std::size_t I, std::size_t J> requires (I < N and J < M)
        constexpr T get() const { return data.at(I * M + J); }
    };
    template <concepts::Numeric T>
    using Mat2 = Mat<2, 2, T>;
    template <concepts::Numeric T>
    using Mat3 = Mat<3, 3, T>;
    template <concepts::Numeric T>
    using Mat4 = Mat<4, 4, T>;

    //: row vectors
    template <concepts::Numeric T>
    using RVec2 = Mat<1, 2, T>;
    template <concepts::Numeric T>
    using RVec3 = Mat<1, 3, T>;

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