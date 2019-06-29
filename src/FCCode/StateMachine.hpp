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
class StateMachine : public StateFieldRegistryReader<bool> {
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
     * @brief Checks if provided state is a valid state. The
     * valid states are defined in the protected state variable field.
     * 
     * @param state The state to check
     * @return Whether or not the state is valid
     */
    bool is_valid_state(unsigned int state) const;

    /**
     * @brief Initialize state to the specified state.
     * 
     * @param initial_state State to set the state machine in.
     * @return True if succeeded, false if state variable is not initialized.
     */
    virtual bool init(unsigned int initial_state);

    /**
     * @brief Forcibly set the state of the state machine to a given state.
     * 
     * @param initial_state State to set the state machine in.
     * @return True if succeeded, false if state variable is not initialized.
     */
    bool set_state(unsigned int state);

    /**
     * @brief Add a ControlTask handler that executes upon every call
     * of update_state_machine() when the current state is equal to
     * the specified state.
     * 
     * @param state 
     * @param handler
     * @return True if succeeded, false if state variable is not initialized.
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
     * @return True if succeeded, false if state variable is not initialized.
     */
    void register_transition_handler(unsigned int state_1, 
                                     unsigned int state_2, 
                                     TransitionHandler& handler);

    /**
     * @brief Run a full update cycle on the state machine, which means running
     * the state handler for the current state, getting the value of the next state,
     * running the pre-transition handler and setting the next state.
     * 
     * @return True if succeeded, false if state variable is not initialized.
     */
    bool update_state_machine();
    bool execute() override;

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
bool StateMachine<compressed_state_sz>::is_valid_state(unsigned int state) const {
    return state < _state._state_names.size();
}

template<size_t compressed_state_sz>
bool StateMachine<compressed_state_sz>::init(unsigned int initial_state) {
    static_assert(is_valid_state(initial_state), "Invalid state number passed into function.");

    return set_state(initial_state);
}

template<size_t compressed_state_sz>
bool StateMachine<compressed_state_sz>::set_state(unsigned int state) {
    static_assert(is_valid_state(state), "Invalid state number passed into function.");

    if(!_state.set(state)) return false;
    StateHandler* state_handler = _state_handlers.at(state);
    state_handler->has_executed = false;
    
    return true;
}

template<size_t compressed_state_sz>
void StateMachine<compressed_state_sz>::register_state_handler(unsigned int state, StateHandler& handler) {
    static_assert(is_valid_state(state), "Invalid state number passed into function.");

     _transition_handlers.emplace(state, &handler);
}

template<size_t compressed_state_sz>
void StateMachine<compressed_state_sz>::register_transition_handler(unsigned int state_1, 
                                                                    unsigned int state_2, 
                                                                    TransitionHandler& handler) {
    static_assert(is_valid_state(state_1), "Invalid state number passed into function.");
    static_assert(is_valid_state(state_1), "Invalid state number passed into function.");
    if (state_1 == state_2) {
        _dbg_console.println(debug_severity::ERROR, 
            "Should not be setting a transition handler between two of the same state.");
        return;
    }

    std::pair<unsigned int, unsigned int> state_pair(state_1, state_2);
    _transition_handlers.emplace(state_pair, &handler);
}

template<size_t compressed_state_sz>
bool StateMachine<compressed_state_sz>::update_state_machine() {
    // Get current state and state handler
    unsigned int cur_state = _state.get();
    static_assert(is_valid_state(cur_state), "Invalid state number for current state.");

    StateHandler* state_handler = nullptr;
    try {
        StateHandler* state_handler = _state_handlers.at(cur_state);
    }
    catch (std::out_of_range& err) {
        _dbg_console.printf(debug_console::ERROR, "No state handler specified for state %d!", cur_state);
        return false;
    }

    // Get next state and run transition handler
    unsigned int next_state = cur_state;
    if (!state_handler->only_execute_once || !state_handler->has_executed) {
        next_state = state_handler->execute();
        state_handler->has_executed = true;

        static_assert(is_valid_state(next_state), "Invalid state number for next state.");
        std::pair<unsigned int, unsigned int> state_pair(cur_state, next_state);
        TransitionHandler* transition_handler = _transition_handlers.at(state_pair);
        transition_handler->execute();
        // Set current state equal to next state
        return set_state(next_state);
    }
    return true;
}

template<size_t compressed_state_sz>
bool StateMachine<compressed_state_sz>::execute() {
    return update_state_machine();
}

#endif