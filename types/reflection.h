//* reflection
//      compile time type reflection
//      - citations and explanation
#pragma once

#include "std_types.h"

#ifndef __cpp_lib_is_aggregate
#error "aggregate detection not supported"
#endif

namespace fresa
{
    namespace concepts
    {
        //* aggregate type
        template <typename T>
        concept Aggregate = std::is_aggregate_v<T>;

        //* aggregate initializable
        template <typename T, typename ... Args>
        concept AggregateInitializable = Aggregate<T> and requires { T{std::declval<Args>()...}; };

        //* aggregate initializable with n arguments
        namespace detail
        {
            struct Any { 
                template <typename T> 
                constexpr operator T() const noexcept;
            };

            template <std::size_t I>
            using IndexedAny = Any;

            template <Aggregate T, typename I>
            struct AggregateFromIndices;

            template <Aggregate T, std::size_t ... I>
            struct AggregateFromIndices<T, std::index_sequence<I...>> : std::bool_constant<AggregateInitializable<T, IndexedAny<I>...>> {};
        }
        template <typename T, std::size_t N>
        concept AggregateN = Aggregate<T> and detail::AggregateFromIndices<T, std::make_index_sequence<N>>::value;
    }

    //* count struct fields
    namespace detail
    {
        template <concepts::Aggregate T, std::size_t N, bool CanInitialize>
        struct field_count : field_count<T, N+1, concepts::AggregateN<T, N+1>> {};
        template <concepts::Aggregate T, std::size_t N>
        struct field_count<T, N, false> : std::integral_constant<std::size_t, N-1> {};
    }
    template <concepts::Aggregate T>
    struct field_count : detail::field_count<T, 0, true> {};
    template <concepts::Aggregate T>
    constexpr std::size_t field_count_v = field_count<T>::value;
}