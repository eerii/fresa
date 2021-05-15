//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include <tuple>
#include <variant>

#include "dtypes.h"
#include "arrays.h"
#include "log.h"

namespace Verse::State
{

template <typename... States>
struct StateMachine {
    //Constructor
    StateMachine() = default;
    StateMachine(States... p_states) : states(std::move(p_states)...) {};
    
    //All possible states
    std::tuple<States...> states;
    
    //Current state (we select the first one as the initial state)
    std::variant<States*...> curr_state { &std::get<0>(states) };
    
    //Handle events
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
    
    //Change state
    template <typename State>
    State& transition_to() {
        State& state = std::get<State>(states);
        curr_state = &state;
        return state;
    }
    
};

//Intermediate object to avoid tying the states together
template <typename TargetState>
struct To {
    template <typename Machine, typename State, typename Event>
    void execute (Machine& machine, State& prev_state, const Event& event) {
        leave(prev_state, event);
        TargetState& new_state = machine.template transition_to<TargetState>();
        enter(new_state, event);
    }
    
    //On leave
    void leave(...) {}
    template <typename State, typename Event>
    auto leave(State& state, const Event& event) -> decltype(state.onLeave(event)) {
        return state.onLeave(event);
    }
    
    //On enter
    void enter(...) {}
    template <typename State, typename Event>
    auto enter(State& state, const Event& event) -> decltype(state.onEnter(event)) {
        return state.onEnter(event);
    }
};

//Ignore the event passed
struct Nothing {
    template <typename Machine, typename State, typename Event>
    void execute (Machine&, State&, const Event&) {}
};

//Default action
template <typename Action>
struct Default {
    template <typename Event>
    Action handle(const Event&) const {
        return Action();
    }
};

//Only transition to next state
template <typename Event, typename Action>
struct On {
    Action handle(const Event&) const {
        return {};
    }
};

//Define the state
template <typename... Handlers>
struct Do : Handlers... {
    using Handlers::handle...;
};

//Have a list of actions and choose one to execute
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

//For convenience, a wrapper for the previous function, for doing one action or nothing
template <typename Action>
struct ThisOrNothing : OneOf<Action, Nothing> {
    using OneOf<Action, Nothing>::OneOf;
};

}
