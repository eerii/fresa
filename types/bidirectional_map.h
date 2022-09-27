//* bidirectional map
//      this file provides a bidirectional mapping between two types
//      it is recommended to use this with simple indexing types, because two copies of them are stored
//      it can be used for example to map attachments, subpasses and renderpasses with one another
#pragma once

#include "std_types.h"
#include <set>

namespace fresa
{
    template <typename A, typename B>
    struct BiMap {
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
}