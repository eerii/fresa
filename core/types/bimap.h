//project verse, 2017-2022
//by jose pazos perez
//licensed under GPLv3

#pragma once

#include "types.h"

namespace Fresa
{
    template <typename A, typename B>
    struct bi_map_AB_BA {
        std::map<A, B> a_to_b{};
        std::map<B, A> b_to_a{};
        
        void add(A a, B b) {
            if (a_to_b.count(a))
                log::error("Adding a repeated A key to a bi map that doesn't support it");
            if (b_to_a.count(b))
                log::error("Adding a repeated B key to a bi map that doesn't support it");
            a_to_b[a] = b;
            b_to_a[b] = a;
        }
    };
    
    template <typename A, typename B>
    struct bi_map_AvB_BA {
        std::map<A, std::vector<B>> a_to_b{};
        std::map<B, A> b_to_a{};
        
        void add(A a, B b) {
            if (b_to_a.count(b))
                log::error("Adding a repeated B key to a bi map that doesn't support it");
            a_to_b[a].push_back(b);
            b_to_a[b] = a;
        }
    };
    
    template <typename A, typename B>
    struct bi_map_AvB_BvA {
        std::map<A, std::vector<B>> a_to_b{};
        std::map<B, std::vector<A>> b_to_a{};
        
        void add(A a, B b) {
            a_to_b[a].push_back(b);
            b_to_a[b].push_back(a);
        }
    };
    
    template<typename T> struct is_vector { static constexpr bool value=false; };
    template<typename T> struct is_vector<std::vector<T>> { static constexpr bool value=true; };
    
    template <typename B, typename A, typename M>
    auto getBimapAtoB(A a_id, M &bi_map){
        if (not bi_map.a_to_b.count(a_id))
            log::error("Accessing bi map before it is available.");
        return bi_map.a_to_b.at(a_id);
    };
    
    template <typename B, typename A, typename M, typename BData>
    auto getBimapAtoB_v(A a_id, M &bi_map, const std::map<B, BData> &b_data){
        auto b_keys = getBimapAtoB<B>(a_id, bi_map);
        
        std::map<B, const BData&> b_map{};
        
        if constexpr (is_vector<decltype(b_keys)>::value) {
            for (B &b : b_keys)
                b_map.insert({b, b_data.at(b)});
        } else {
            b_map.insert({b_keys, b_data.at(b_keys)});
        }
        
        return b_map;
    };
    
    template <typename A, typename B, typename M>
    auto getBimapBtoA(B b_id, M &bi_map){
        if (not bi_map.b_to_a.count(b_id))
            log::error("Accessing bi map before it is available.");
        return bi_map.b_to_a.at(b_id);
    };
    
    template <typename A, typename B, typename M, typename AData>
    auto getBimapBtoA_v(B b_id, M &bi_map, const std::map<A, AData> &a_data){
        auto a_keys = getBimapBtoA<A>(b_id, bi_map);
        
        std::map<A, const AData&> a_map{};
        
        if constexpr (is_vector<decltype(a_keys)>::value) {
            for (A &a : a_keys)
                a_map.insert({a, a_data.at(a)});
        } else {
            a_map.insert({a_keys, a_data.at(a_keys)});
        }
        
        return a_map;
    };
}
