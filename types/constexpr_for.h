//* constexpr for
//      this file provides a set of constexpr for loops for iterating using integral constants or parameters
//      the implementation is derived from this article https://artificial-mind.net/blog/2020/10/31/constexpr-for
#pragma once

#include <tuple>

namespace fresa
{
    //: helper concepts for detecting tuple like objects
    namespace concepts
    {
        namespace detail
        {
            template <typename T, std::size_t N>
            concept HasTupleElement = requires(T t) {
                typename std::tuple_element_t<N, std::remove_const_t<T>>;
                { get<N>(t) } -> std::convertible_to<const std::tuple_element_t<N, T>&>;
            };
        }

        template <typename T>
        concept TupleLike = requires (T t) {
            typename std::tuple_size<T>::type;
        } and []<std::size_t ... N>(std::index_sequence<N...>) {
            return (detail::HasTupleElement<T, N> && ...);
        }(std::make_index_sequence<std::tuple_size_v<T>>());
    }
    
    //: integral loop (i = 0; i < n; i++)
    template <auto From, auto To, auto Increment = 1, typename F>
    constexpr void for_(F&& f) {
        if constexpr (From < To) {
            using T = decltype(f(std::integral_constant<decltype(From), From>()));
            if constexpr (std::same_as<T, void>) {
                f(std::integral_constant<decltype(From), From>());
            } else if constexpr (std::convertible_to<T, bool>) {
                if (not f(std::integral_constant<decltype(From), From>()))
                    return;
            }
            for_<From + Increment, To, Increment>(f);
        }
    }

    //: parameter pack loop (auto a : args...)
    template <typename F, typename ... A>
    constexpr void for_(F&& f, A&& ... args) {
        (f(std::forward<A>(args)), ...);
    }

    //: tuple like loop (auto a : tuple_object)
    template <typename F, concepts::TupleLike T>
    constexpr void for_(F&& f, T&& t) {
        constexpr std::size_t N = std::tuple_size_v<std::decay_t<T>>;
        for_<0, N>([&](auto i) {
            f(std::get<i>(t));
        });
    }
}

