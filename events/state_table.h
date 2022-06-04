//: fresa by jose pazos perez, licensed under GPLv3
#pragma once

#include "state_machine.h"

//---State tables---
//      Generates compile time state tables for state machines

namespace Fresa::State
{

    //: First row and column
    struct Header {};

    //: Wrappers to generate the strings
    struct MakeString {
        constexpr auto operator()(Types<Header>) const {
            return Str{""};
        }
        
        template <typename T>
        constexpr auto operator()(Types<T> t) const {
            return make_string(t).data();
        }
    };
    template <std::size_t W>
    struct MakeStringConstantW {
        constexpr auto operator()(Types<Header>) const {
            return Str{""}.template changeLength<W>(' ');
        }

        template <typename T>
        constexpr auto operator()(Types<T> t) const {
            return make_string(t).template changeLength<W>(' ');
        }
    };

    //: Generate row
    template <typename MakeString, typename State>
    struct GenerateRow {
        const MakeString str;
        
        constexpr GenerateRow(MakeString p_str, Types<State>) : str(p_str) {}
        
        constexpr auto operator()(Types<State> state) const {
            return str(state);
        }
        
        template <typename Event>
        constexpr auto operator()(Types<Event>) const {
            auto action = ResolveAction{}(Types<Types<State, Event>>{});
            return Str{" | "} + str(action);
        }
    };

    //: Generate header row
    template <typename MakeString>
    struct GenerateRow<MakeString, Header> {
        const MakeString str;
        
        constexpr GenerateRow(MakeString p_str, Types<Header>) : str(p_str) {}
        
        constexpr auto operator()(Types<Header> header) const {
            return str(header);
        }
        
        template <typename Event>
        constexpr auto operator()(Types<Event> event) const {
            return Str{" | "} + str(event);
        }
    };

    //: Make table
    template <typename MakeString, typename... Events>
    struct GenerateTable {
        const MakeString str;
        
        constexpr GenerateTable(MakeString p_str, Types<Events...>) : str(p_str) {}
        
        template <typename State>
        constexpr auto operator()(Types<State> state) const {
            return (Types<State, Events...>{} | MapAndSum{GenerateRow{str, state}}) + Str{"\n"};
        }
    };

    //: Make transition table
    template <typename... StateTypes, typename... EventTypes>
    constexpr auto generateTransitionTable(Types<StateTypes...> states, Types<EventTypes...> events) {
        constexpr MakeString str;
        constexpr auto table = (Types<Header>{} + states) | MapAndSum{GenerateTable{str, events}};
        return table;
    }

    //: Calculate the length of the largest cell
    template <std::size_t X>
    struct Max {
        template <std::size_t Y>
        constexpr auto operator+(Max<Y>) {
            return Max<std::max(X, Y)>{};
        }

        static constexpr auto value() {
            return X;
        }
    };

    struct CalculateMaxLength {
        template <typename T>
        constexpr auto operator()(Types<T> type) {
            return Max<make_string(type).length()>{};
        }
    };

    //: Make formatted transition table
    template <typename... StateTypes, typename... EventTypes>
    constexpr auto generatePrettyTransitionTable(Types<StateTypes...> states, Types<EventTypes...> events) {
        constexpr auto actions = (states * events) | MapAndSum(ResolveAction{});
        constexpr auto max_w = (states + events + actions) | MapAndSum(CalculateMaxLength{});
        
        constexpr MakeStringConstantW<max_w.value()> str{};
        constexpr auto table = (Types<Header>{} + states) | MapAndSum{GenerateTable{str, events}};
        return table;
    }

}
