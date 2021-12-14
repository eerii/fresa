//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#include <variant>

namespace Fresa
{
    //---Check if it is variant type---
    template<typename T> struct is_variant : std::false_type {};

    template<typename ...Args>
    struct is_variant<std::variant<Args...>> : std::true_type {};

    template<typename T>
    inline constexpr bool is_variant_v=is_variant<T>::value;
    
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
}
