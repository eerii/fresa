//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

namespace Verse
{
    template <typename... T>
    struct Types {};

    template <typename... L, typename... R>
    constexpr auto operator+(Types<L...>, Types<R...>) {
        return Types<L..., R...>{};
    }


    //CARTESIAN PRODUCT (Domain of state function)
    template <typename L, typename...R>
    constexpr auto operator*(Types<L>, Types<R...>) {
        return Types<Types<L, R>...>{};
    }
    template <typename... L, typename R>
    constexpr auto operator*(Types<L...>, R r) {
        return ((Types<L>{} * r) + ...);
    }

    //Operate on every type
    template <typename... T, typename Operation>
    constexpr auto operator|(Types<T...>, Operation op) {
        return op(Types<T>{}...);
    }

    //Apply operation and sum
    template <typename Operation>
    struct MapAndSum {
        Operation op;
        constexpr MapAndSum(Operation p_op) : op(std::move(p_op)) {};
        
        template <typename... T>
        constexpr auto operator()(Types<T>... t) {
            return (op(t) + ...);
        }
    };
}
