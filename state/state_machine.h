//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#include <tuple>
#include <variant>
#include <typeinfo>

#include "static_str.h"

#ifndef __EMSCRIPTEN__
#define CURR_STATE(SM) std::visit([](auto&&x) { \
    using T = std::decay_t<decltype(*x)>; \
    return make_string(Types<T>()).c_str(); \
}, SM.curr_state)
#else
#define CURR_STATE(SM) ""
#endif

//---State machines---
//      First attempt at an implementation of state machines, has room for improvement
//      All the table descriptions are created at compile time so it should be really fast during runtime (however, it does take extra time to
//      compile when working with state machines, but the benefits are worth it in my opinion)

namespace Fresa::State
{
    //State Types
    //-------------------------------------
    template <typename... T>
    struct Types {};

    template <typename... L, typename... R>
    constexpr auto operator+(Types<L...>, Types<R...>) {
        return Types<L..., R...>{};
    }

    //: Cartesian Product (Domain of state function)
    template <typename L, typename...R>
    constexpr auto operator*(Types<L>, Types<R...>) {
        return Types<Types<L, R>...>{};
    }
    template <typename... L, typename R>
    constexpr auto operator*(Types<L...>, R r) {
        return ((Types<L>{} * r) + ...);
    }

    //: Operate on every type
    template <typename... T, typename Operation>
    constexpr auto operator|(Types<T...>, Operation op) {
        return op(Types<T>{}...);
    }
    //-------------------------------------

    //Apply operation and sum
    //-------------------------------------
    template <typename Operation>
    struct MapAndSum {
        Operation op;
        constexpr MapAndSum(Operation p_op) : op(std::move(p_op)) {};
        
        template <typename... T>
        constexpr auto operator()(Types<T>... t) {
            return (op(t) + ...);
        }
    };
    //-------------------------------------

    //State Machine
    //-------------------------------------
    template <typename... States>
    struct StateMachine {
        //: Constructor
        StateMachine() = default;
        StateMachine(States... p_states) : states(std::move(p_states)...) {};
        
        //: All possible states
        std::tuple<States...> states;
        
        void updateStates(States... p_states) {
            states = std::make_tuple(std::move(p_states)...);
        }
        
        //: Current state (we select the first one as the initial state)
        std::variant<States*...> curr_state { &std::get<0>(states) };
        
        //: Handle events
        template <typename Event>
        void handle(const Event& event) {
            handleBy(event, *this);
        }
        template <typename Event, typename Machine>
        void handleBy(const Event& event, Machine& machine) {
            auto pass_to_state = [&machine, &event] (auto state_ptr) {
                auto action = state_ptr->handle(event);
                action.execute(machine, *state_ptr, event);
            };
            std::visit(pass_to_state, curr_state);
        }
        
        //: Get type
        std::type_info const& curr_type(){
          return std::visit( [](auto&&x)->decltype(auto){ return typeid(*x); }, curr_state );
        }
        template <typename State>
        bool is(const State& state) {
            return curr_type() == typeid(state);
        }
        
        //: Change state
        template <typename State>
        State& transition_to() {
            State& state = std::get<State>(states);
            curr_state = &state;
            return state;
        }
        
        //: State types
        constexpr static Types<States...> getStateTypes() { return {}; };
    };
    //-------------------------------------

    //Intermediate objects to avoid tying the states together
    //-------------------------------------
    template <typename TargetState>
    struct To {
        template <typename Machine, typename State, typename Event>
        void execute (Machine& machine, State& prev_state, const Event& event) {
            leave(prev_state, event);
            TargetState& new_state = machine.template transition_to<TargetState>();
            enter(new_state, event);
        }
        
        //: On leave
        void leave(...) {}
        template <typename State, typename Event>
        auto leave(State& state, const Event& event) -> decltype(state.onLeave(event)) {
            return state.onLeave(event);
        }
        
        //: On enter
        void enter(...) {}
        template <typename State, typename Event>
        auto enter(State& state, const Event& event) -> decltype(state.onEnter(event)) {
            return state.onEnter(event);
        }
    };

    //: Ignore the event passed
    struct Nothing {
        template <typename Machine, typename State, typename Event>
        void execute (Machine&, State&, const Event&) {}
    };

    //: Default action
    template <typename Action>
    struct Default {
        template <typename Event>
        Action handle(const Event&) const {
            return Action();
        }
    };

    //: Only transition to next state
    template <typename Event, typename Action>
    struct On {
        Action handle(const Event&) const {
            return {};
        }
    };

    //: Define the state
    template <typename... Handlers>
    struct Do : Handlers... {
        using Handlers::handle...;
    };

    //: Have a list of actions and choose one to execute
    template <typename... Actions>
    struct OneOf {
        std::variant<Actions...> options;
        
        template <typename T>
        OneOf(T&& param) : options(std::forward<T>(param)) {};
        
        template <typename Machine, typename State, typename Event>
        void execute(Machine& machine, State& state, const Event& event) {
            std::visit([&machine, &state, &event](auto& action){ action.execute(machine, state, event); }, options);
        }
    };

    //: For convenience, a wrapper for the previous function, for doing one action or nothing
    template <typename Action>
    struct Maybe : OneOf<Action, Nothing> {
        using OneOf<Action, Nothing>::OneOf;
    };

    //: Convert state-event pairs and turn them into an action
    struct ResolveAction {
        template <typename State, typename Event>
        constexpr auto operator()(Types<State, Event>) {
            using Action = decltype(std::declval<State>().handle(std::declval<Event>()));
            return Types<Action>{};
        }
        
        template <typename State, typename Event>
        constexpr auto operator()(Types<Types<State, Event>>) {
            return (*this)(Types<State, Event>{});
        }
    };
    //-------------------------------------

}
