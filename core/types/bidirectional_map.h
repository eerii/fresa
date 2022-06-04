//: fresa by jose pazos perez, licensed under GPLv3
#pragma once

#include "types.h"
#include <set>

namespace Fresa
{
    template <typename A, typename B>
    struct map_AB_BA {
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
    struct map_AvB_BA {
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
    struct map_AvB_BvA {
        std::map<A, std::vector<B>> a_to_b{};
        std::map<B, std::vector<A>> b_to_a{};
        
        void add(A a, B b) {
            a_to_b[a].push_back(b);
            b_to_a[b].push_back(a);
        }
    };
    
    template <typename MapAX, typename MapXB>
    struct map_chain {
        MapAX &ax;
        MapXB &xb;
        
        map_chain(MapAX &m_a, MapXB &m_b) : ax(m_a), xb(m_b) {};
    };
    
    template<typename T> struct is_mapAB { static constexpr bool value=false; };
    template<typename A, typename B> struct is_mapAB<map_AB_BA<A,B>> { static constexpr bool value=true; };
    template<typename A, typename B> struct is_mapAB<map_AvB_BA<A,B>> { static constexpr bool value=true; };
    template<typename A, typename B> struct is_mapAB<map_AvB_BvA<A,B>> { static constexpr bool value=true; };
    
    template<typename T> struct is_chain { static constexpr bool value=false; };
    template<typename AX, typename XB> struct is_chain<map_chain<AX,XB>> { static constexpr bool value=true; };
    
    template <typename B, typename A, typename Map, std::enable_if_t<is_mapAB<Map>::value or is_chain<Map>::value, bool> = true>
    auto getAtoB(A a, Map &map) {
        //---Simple map (A -> B)---
        if constexpr (is_mapAB<Map>::value) {
            if (not map.a_to_b.count(a)) { //: There is no A key
                log::error("Accessing bidirectional map with an invalid A key");
                if constexpr (std::is_same_v<Map, map_AB_BA<A,B>>) { return B{}; } //: The map returns a single B (A -> B)
                else { return std::vector<B>{}; } //: The map returns a list of B (A -> vB)
            }
            return map.a_to_b.at(a);
        //---Chain (A -> X -> B)---
        } else {
            if (not map.ax.a_to_b.count(a)) //: There is no A key
                log::error("Accessing map chain with an invalid A key is forbidden");
            auto x = map.ax.a_to_b.at(a);
            if constexpr (not is_vector_v<decltype(x)>) { //: Single value of X (A -> X)
                return getAtoB<B>(x, map.xb);
            } else { //: List of X (A -> vX)
                std::set<B> b_list;
                for (auto i : x) {
                    auto b = getAtoB<B>(i, map.xb);
                    if constexpr (not is_vector_v<decltype(b)>) { //: Single value of B (A -> vX -> B)
                        b_list.insert(b);
                    } else { //: List of B (A -> vX -> vB)
                        std::copy(b.begin(), b.end(), std::inserter(b_list, b_list.end()));
                    }
                }
                return std::vector<B>{b_list.begin(), b_list.end()};
            }
        }
    }
    
    template <typename A, typename B, typename Map, std::enable_if_t<is_mapAB<Map>::value or is_chain<Map>::value, bool> = true>
    auto getBtoA(B b, Map &map) {
        //---Simple map (B -> A)---
        if constexpr (is_mapAB<Map>::value) {
            if (not map.b_to_a.count(b)) { //: There is no B key
                log::error("Accessing bidirectional map with an invalid B key");
                if constexpr (not std::is_same_v<Map, map_AvB_BvA<A,B>>) { return A{}; } //: The map returns a single A (B -> A)
                else { return std::vector<A>{}; } //: The map returns a list of A (B -> vA)
            }
            return map.b_to_a.at(b);
        //---Chain (B -> X -> A)---
        } else {
            if (not map.xb.b_to_a.count(b)) //: There is no B key
                log::error("Accessing map chain with an invalid B key is forbidden");
            auto x = map.xb.b_to_a.at(b);
            if constexpr (not is_vector_v<decltype(x)>) { //: Single value of X (B -> X)
                return getBtoA<A>(x, map.ax);
            } else { //: List of X (B -> vX)
                std::set<A> a_list;
                for (auto i : x) {
                    auto a = getBtoA<A>(i, map.ax);
                    if constexpr (not is_vector_v<decltype(a)>) { //: Single value of A (B -> vX -> A)
                        a_list.insert(a);
                    } else { //: List of A (B -> vX -> vA)
                        std::copy(a.begin(), a.end(), std::inserter(a_list, a_list.end()));
                    }
                }
                return std::vector<A>{a_list.begin(), a_list.end()};
            }
        }
    }
    
    template <typename B, typename A, typename Map, typename BData, std::enable_if_t<is_mapAB<Map>::value or is_chain<Map>::value, bool> = true>
    auto getAtoB_v(A a, Map &map, const std::map<B, BData> &b_data) {
        auto b = getAtoB<B>(a, map);
        
        if constexpr (not is_vector_v<decltype(b)>) { //: Single value of B (A -> ... -> B)
            return std::pair<B, const BData&>{b, b_data.at(b)};
        } else { //: List of B (A -> ... -> vB)
            std::map<B, const BData&> b_map{};
            for (auto i : b)
                b_map.insert(b_map.end(), std::pair<B, const BData&>{i, b_data.at(i)});
            return b_map;
        }
    }
    
    template <typename A, typename B, typename Map, typename AData, std::enable_if_t<is_mapAB<Map>::value or is_chain<Map>::value, bool> = true>
    auto getBtoA_v(B b, Map &map, const std::map<A, AData> &a_data) {
        auto a = getBtoA<A>(b, map);
        
        if constexpr (not is_vector_v<decltype(a)>) { //: Single value of A (B -> ... -> A)
            return std::pair<A, const AData&>{a, a_data.at(a)};
        } else { //: List of A (B -> ... -> vA)
            std::map<A, const AData&> a_map{};
            for (auto i : a)
                a_map.insert(a_map.end(), std::pair<A, const AData&>{i, a_data.at(i)});
            return a_map;
        }
    }
}
