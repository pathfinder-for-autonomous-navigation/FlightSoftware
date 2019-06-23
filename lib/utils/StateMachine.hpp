#ifndef STATE_MACHINE_HPP_
#define STATE_MACHINE_HPP_

#include <type_traits>
#include <map>
#include <string>
#include <utility>
#include "ControlTask.hpp"
#include "StateField.hpp"
#include "StateMachineStateField.hpp"
#include <Debuggable.hpp>

/**
 * @brief A generic class for a state machine controller.
 */
template<size_t compressed_state_sz>
class StateMachine : public StateFieldRegistryReader<void> {
  public:
    /**
     * @brief Construct a new State Machine object
     * 
     * @param sv Variable that contains the state upon which this state machine acts.
     * @param dbg_console Console for outputting state machine error messages, if any.
     */
    StateMachine(SMStateField<compressed_state_sz>& sv,
                 const std::string& name,
                 debug_console& dbg_console,
                 StateFieldRegistry& r);

    /**
     * @brief Initialize state to the specified state.
     * 
     * @param initial_state State to set the state machine in.
     */
    void init(unsigned int initial_state);

    /**
     * @brief Forcibly set the state of the state machine to a given state.
     * 
     * @param initial_state State to set the state machine in.
     */
    void set_state(unsigned int state);

    /**
     * @brief Add a ControlTask handler that executes upon every call
     * of update_state_machine() when the current state is equal to
     * the specified state.
     * 
     * @param state 
     * @param handler 
     */
    void register_state_handler(unsigned int state, StateHandler& handler);

    /**
     * @brief Add a ControlTask handler that executes prior to a transition
     * from state 1 to state 2. The state transition happens within update_state_machine,
     * after the state handler has already been called.
     * 
     * @param state_1
     * @param state_2
     * @param handler
     */
    void register_transition_handler(unsigned int state_1, 
                                     unsigned int state_2, 
                                     TransitionHandler& handler);

    /**
     * @brief Run a full update cycle on the state machine, which means running
     * the state handler for the current state, getting the value of the next state,
     * running the pre-transition handler and setting the next state.
     */
    void update_state_machine();
    void execute() override;

  protected:
    SMStateField<compressed_state_sz>& _state;
    std::map<unsigned int, StateHandler*> _state_handlers;
    std::map<std::pair<unsigned int, unsigned int>, TransitionHandler*> _transition_handlers;
};

template<size_t compressed_state_sz>
StateMachine<compressed_state_sz>::StateMachine(SMStateField<compressed_state_sz>& sv,
                                                const std::string& name,
                                                debug_console& dbg_console,
                                                StateFieldRegistry& r) : 
        StateFieldRegistryReader(name, dbg_console, r),
        _state(sv) {
    if (can_read(sv))
        dbg_console.printf(debug_severity::WARNING,
            "State machine %s does not have read access to its own state variable %s.", 
            name.c_str(),
            sv.name().c_str());
    if (can_write(sv))
        dbg_console.printf(debug_severity::WARNING,
            "State machine %s does not have write access to its own state variable %s.", 
            name.c_str(),
            sv.name().c_str());
}

template<size_t compressed_state_sz>
void StateMachine<compressed_state_sz>::init(unsigned int initial_state) {
    set_state(initial_state);
}

template<size_t compressed_state_sz>
void StateMachine<compressed_state_sz>::set_state(unsigned int state) {
    _state.set(state);
}

template<size_t compressed_state_sz>
void StateMachine<compressed_state_sz>::register_state_handler(unsigned int state, StateHandler& handler) {
     _transition_handlers.emplace(state, &handler);
}

template<size_t compressed_state_sz>
void StateMachine<compressed_state_sz>::register_transition_handler(unsigned int state_1, 
                                                                    unsigned int state_2, 
                                                                    TransitionHandler& handler) {
    std::pair<unsigned int, unsigned int> state_pair(state_1, state_2);
    _transition_handlers.emplace(state_pair, &handler);
}

template<size_t compressed_state_sz>
void StateMachine<compressed_state_sz>::update_state_machine() {
    // Get current state and state handler
    unsigned int cur_state = _state.get();
    StateHandler* state_handler = _state_handlers.at(cur_state);

    // Get next state and run pre-transition handler
    unsigned int next_state = state_handler->execute();
    std::pair<unsigned int, unsigned int> state_pair(cur_state, next_state);
    TransitionHandler* transition_handler = _transition_handlers.at(state_pair);
    transition_handler->execute();
    
    // Set current state equal to next state
    set_state(next_state);
}

template<size_t compressed_state_sz>
void StateMachine<compressed_state_sz>::execute() {
    update_state_machine();
}

#endif