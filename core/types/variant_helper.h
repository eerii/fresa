//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#include <variant>
#include <tuple>

namespace Fresa
{
    //---Check if it is variant type---
    template<typename T> struct is_variant : std::false_type {};

    template<typename ...Args>
    struct is_variant<std::variant<Args...>> : std::true_type {};

    template<typename T>
    inline constexpr bool is_variant_v=is_variant<T>::value;
    
    //---Variant from tuple---
    template <typename... Ts>
    std::variant<Ts...> variant_from_tuple (std::tuple<Ts...>);
    
    template <typename... Ts>
    std::tuple<Ts...> tuple_from_variant (std::variant<Ts...>);
    
    //---Filter variant---
    template <template <typename> class C, typename T>
    std::enable_if_t<true == C<T>::value, std::tuple<>> filter_t ();

    template <template <typename> class C, typename T>
    std::enable_if_t<false == C<T>::value, std::tuple<T>> filter_t ();
    
    template <template <typename> class C, typename... Args>
    struct filtered_t {
        using value = decltype(std::tuple_cat(filter_t<C, Args>()...));
    };
    
    template <template <typename> class C, typename... Args>
    struct filter_ {
        using value = decltype(variant_from_tuple(std::declval<typename filtered_t<C, Args...>::value>()));
    };
    
    //---Constexpr for---
    template<std::size_t N>
    struct num { static const constexpr auto value = N; };

    template <class F, std::size_t... Is>
    void for_(F func, std::index_sequence<Is...>) {
        (func(num<Is>{}), ...);
    }

    template <std::size_t N, typename F>
    void for_(F func) {
        for_(func, std::make_index_sequence<N>());
    }
    
    template <typename V, typename F, std::enable_if_t<is_variant_v<V>, bool> = true>
    void for_(F func) {
        for_<std::variant_size_v<V>>(func);
    }
    
    //---Get type index in variant---
    template <typename> struct tag { };

    template <typename T, typename V>
    struct getVariantIndex;

    template <typename T, typename... Ts>
    struct getVariantIndex<T, std::variant<Ts...>> : std::integral_constant<size_t, std::variant<tag<Ts>...>(tag<T>()).index()> { };
    
    //---Combine two variants---
    template <typename T, typename... Args> struct concatenate_;

    template <typename... Args0, typename... Args1>
    struct concatenate_<std::variant<Args0...>, std::variant<Args1...>> {
        using type = std::variant<Args0..., Args1...>;
    };
}
