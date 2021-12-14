//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

namespace Fresa
{
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
}
