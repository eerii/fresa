//* bidirectional map
//      this file provides a bidirectional mapping between two types
//      it is recommended to use this with simple indexing types, because two copies of them are stored
//      it can be used for example to map attachments, subpasses and renderpasses with one another
#pragma once

#include "constexpr_for.h"
#include "strong_types.h"
#include <set>

namespace fresa
{
    //* bidirectional map definition
    template <typename _A, typename _B>
    struct BiMap {
        using A = _A;
        using B = _B;

        //: constructors
        BiMap() = default;
        BiMap(std::vector<std::pair<A, B>> &&pairs) {
            for (auto& [a, b] : pairs)
                add(a, b);
        }
        
        //: sets to store the mapping
        std::set<std::pair<A, B>> a_to_b{};
        std::set<std::pair<B, A>> b_to_a{};

        //: add a mapping between a and b
        constexpr void add(A a, B b) {
            auto it_a = a_to_b.insert({a, b});
            b_to_a.insert({b, a});
        }

        //: get a range view of all the b's mapped to an a, and viceversa
        constexpr auto get_b(A a) { return a_to_b | rv::filter([a](auto &v){ return v.first == a; }) | rv::values; }
        constexpr auto get_a(B b) { return b_to_a | rv::filter([b](auto &v){ return v.first == b; }) | rv::values; }

        //: remove a specific pair, or all the pairs mapped to an a or b
        constexpr void remove(A a, B b) {
            a_to_b.erase({a, b});
            b_to_a.erase({b, a});
        }
        constexpr void remove_a(A a) {
            for (auto i : get_b(a))
                b_to_a.erase({i, a});
            std::erase_if(a_to_b, [a](auto &v){ return v.first == a; });
        }
        constexpr void remove_b(B b) {
            for (auto i : get_a(b))
                a_to_b.erase({i, b});
            std::erase_if(b_to_a, [b](auto &v){ return v.first == b; });
        }
    };

    //* bidirectional map concept
    namespace concepts
    {
        template <typename T> concept BidirectionalMap = SpecializationOf<T, fresa::BiMap>;
    }

    //* bidimensional chain
    //      a combination of multiple bidirectional maps that allows for mapping between the edges
    template <concepts::BidirectionalMap ... Maps>
    requires (sizeof...(Maps) > 1)
    struct BiChain {
        //: first and last types of the chain
        using A = typename std::tuple_element_t<0, std::tuple<Maps...>>::A;
        using B = typename std::tuple_element_t<sizeof...(Maps) - 1, std::tuple<Maps...>>::B;

        //: constructor, takes references to the chained maps
        //      we also add here a static assert to check that the maps are chained correctly
        BiChain(Maps& ... maps) : maps(maps...) {
            for_<0, sizeof...(Maps) - 1>([](auto i) {
                using MapA = typename std::tuple_element_t<i, std::tuple<Maps...>>;
                using MapB = typename std::tuple_element_t<i + 1, std::tuple<Maps...>>;
                static_assert(std::same_as<typename MapA::B, typename MapB::A>);
            });
        }

        //: saved references to the chained maps
        std::tuple<Maps&...> maps;

        //: get a range view of all the b's mapped to an a and viceversa
        constexpr auto get_b(A a) {
            auto first = std::get<0>(maps).get_b(a);
            return [&]<std::size_t ... I>(std::index_sequence<I...>) {
                return (first | ((rv::transform([this](auto &x){ return std::get<I+1>(maps).get_b(x); }) | rv::join) | ...));
            }(std::make_index_sequence<sizeof...(Maps) - 1>{});
        }
        constexpr auto get_a(B b) {
            auto last = std::get<sizeof...(Maps) - 1>(maps).get_a(b);
            return [&]<std::size_t ... I>(std::index_sequence<I...>) {
                return (last | ((rv::transform([this](auto &x){ return std::get<sizeof...(Maps) - I-2>(maps).get_a(x); }) | rv::join) | ...));
            }(std::make_index_sequence<sizeof...(Maps) - 1>{});
        }
    };
}