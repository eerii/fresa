//* struct fields
//      get the numbers of fields in a struct, uses a method similar to the pfr library
#pragma once

#include <concepts>
#include <utility>

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
    }

    namespace detail
    {
        struct Any { 
            template <typename T> 
            constexpr operator T() const noexcept;
        };

        template <std::size_t I>
        using IndexedAny = Any;

        template <concepts::Aggregate T, typename I>
        struct AggregateFromIndices;

        template <concepts::Aggregate T, std::size_t ... I>
        struct AggregateFromIndices<T, std::index_sequence<I...>> : std::bool_constant<concepts::AggregateInitializable<T, IndexedAny<I>...>> {};
    }

    namespace concepts
    {
        //* aggregate initializable with n arguments
        template <typename T, std::size_t N>
        concept AggregateN = Aggregate<T> and ::fresa::detail::AggregateFromIndices<T, std::make_index_sequence<N>>::value;
    }

    //* binary search (to avoid too many template instantiations)
    namespace detail
    {
        template <template<std::size_t> typename Predicate, std::size_t Begin, std::size_t End>
        struct BinarySearch;

        template <template<std::size_t> typename Predicate, std::size_t Begin, std::size_t End>
        using BinarySearchBase = std::conditional_t<(End - Begin <= 1), std::integral_constant<std::size_t, Begin>,
                                                    std::conditional_t<Predicate<(Begin + End) / 2>::value,
                                                                       BinarySearch<Predicate, (Begin + End) / 2, End>,
                                                                       BinarySearch<Predicate, Begin, (Begin + End) / 2>>>;

        template <template<std::size_t> typename Predicate, std::size_t Begin, std::size_t End>
        struct BinarySearch : BinarySearchBase<Predicate, Begin, End> {};
    }

    //* count struct fields

    //: simple recursive count
    /*
    namespace detail
    {
        template <concepts::Aggregate T, std::size_t N, bool CanInitialize>
        struct RegularFieldCount : RegularFieldCount<T, N+1, concepts::AggregateN<T, N+1>> {};
        template <concepts::Aggregate T, std::size_t N>
        struct RegularFieldCount<T, N, false> : std::integral_constant<std::size_t, N-1> {};
    }
    template <concepts::Aggregate T>
    struct regular_field_count : detail::RegularFieldCount<T, 0, true> {};
    */

    //: binary search
    //      we define the upper bound of the search assuming that each field is only one bit, so there are at most 8 * sizeof(T) fields
    //      this will generally perform better with O(log n) complexity, and will reduce the number of template instantiations
    namespace detail
    {
        template <concepts::Aggregate T>
        struct RegularFieldCount {
            template <std::size_t N>
            struct Initializable : std::bool_constant<concepts::AggregateN<T, N>> {};
        };
    }

    template <concepts::Aggregate T>
    struct regular_field_count : detail::BinarySearch<detail::RegularFieldCount<T>::template Initializable, 0, 8 * sizeof(T) + 1> {};
    
    template <concepts::Aggregate T>
    constexpr std::size_t regular_field_count_v = regular_field_count<T>::value;

    //* array fix
    //      checks whether a field in the aggregate can be initialized with m arguments
    //      useful since regular fields can be initialized with one and only one argument, but arrays can be initialized with more than one argument
    //      this is needed since if you input a struct with a c array, the number of fields is more than it really is
    //      there is an issue with this method, that the objects constructible with a std::initializer_list will always return the maximum number of fields
    namespace detail
    {
        template <concepts::Aggregate T, typename I, typename FieldI>
        struct AggregateIndexInitializableWith;

        template <concepts::Aggregate T, std::size_t... I, std::size_t... FieldI>
        struct AggregateIndexInitializableWith<T, std::index_sequence<I...>, std::index_sequence<FieldI...>> :
               std::bool_constant<requires { T{std::declval<IndexedAny<I>>()..., {std::declval<IndexedAny<FieldI>>()...}}; }> {};
    }

    namespace concepts
    {
        template <typename T, std::size_t N, std::size_t M>
        concept AggregateFieldNWithM = Aggregate<T> and ::fresa::detail::AggregateIndexInitializableWith<T, std::make_index_sequence<N>, std::make_index_sequence<M>>::value;
    }

    namespace detail
    {
        template <concepts::Aggregate T, std::size_t N>
        struct InnerFieldCount {
            template <std::size_t M>
            struct Initializable : std::bool_constant<concepts::AggregateFieldNWithM<T, N, M>> {};
        };
    }

    template <concepts::Aggregate T, std::size_t N>
    struct inner_field_count : detail::BinarySearch<detail::InnerFieldCount<T, N>::template Initializable, 0, 8 * sizeof(T) + 1> {};

    template <concepts::Aggregate T, std::size_t N>
    constexpr std::size_t inner_field_count_v = inner_field_count<T, N>::value;

    //* final count taking into account inner fields
    namespace detail
    {
        template <std::size_t V, std::size_t Total>
        constexpr auto DetectSpecialType = V > Total ? 1 : V;

        template <concepts::Aggregate T, std::size_t Current, std::size_t Total, std::size_t Count>
        struct UniqueFieldCount : UniqueFieldCount<T, Current + DetectSpecialType<inner_field_count_v<T, Current>, Total>, Total, Count + 1> {};
        
        template <concepts::Aggregate T, std::size_t Fields, std::size_t Unique>
        struct UniqueFieldCount<T, Fields, Fields, Unique> : std::integral_constant<std::size_t, Unique> {};
    }

    template <concepts::Aggregate T>
    struct field_count : detail::UniqueFieldCount<T, 0, regular_field_count_v<T>, 0> {};

    template <concepts::Aggregate T>
    constexpr std::size_t field_count_v = field_count<T>::value;
}