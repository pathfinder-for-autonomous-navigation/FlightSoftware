#include "StateMachine.hpp"

template <size_t num_states>
StateMachine<num_states>::StateMachine(const std::string &name, const std::string &state_name,
                                       StateFieldRegistry &r)
    : ControlTask(name, r),
      _state(state_name),
      _state_handlers(),
      _transition_handlers(),
      _state_serializer() {}

template <size_t num_states>
bool StateMachine<num_states>::is_valid_state(unsigned int state) {
    return state < static_cast<unsigned int>(num_states);
}

template <size_t num_states>
bool StateMachine<num_states>::init(const std::array<std::string, num_states> &state_names,
                                    unsigned int initial_state) {
    _state_serializer.init();

    std::shared_ptr<SMStateSerializer<num_states>> serializer_ptr(&_state_serializer);
    _state.init(state_names, serializer_ptr);  // We use the default sanity checker, because
                                               // we already check this value via
                                               // is_valid_state() assertions.

    // TODO give this state machine access to its own state variable.

    if (!is_valid_state(initial_state)) {
        printf(debug_severity::ERROR, "Invalid state number %d passed into function.",
               initial_state);
        return false;
    }

    return set_state(initial_state);
}

template <size_t num_states>
bool StateMachine<num_states>::set_state(unsigned int state) {
    if (!is_valid_state(state)) {
        printf(debug_severity::ERROR, "Invalid state number %d passed into function.", state);
        return false;
    }

    if (!_state.set(std::shared_ptr<ControlTask<bool>>(this), state)) return false;
    std::shared_ptr<StateHandler> state_handler = _state_handlers.at(state);
    state_handler->has_executed = false;

    return true;
}

template <size_t num_states>
void StateMachine<num_states>::register_state_handler(unsigned int state,
                                                      std::shared_ptr<StateHandler> handler) {
    if (!is_valid_state(state))
        printf(debug_severity::ERROR, "Invalid state number %d passed into function.", state);

    _state_handlers.emplace(state, handler);
}

template <size_t num_states>
void StateMachine<num_states>::register_transition_handler(
    unsigned int state_1, unsigned int state_2, std::shared_ptr<TransitionHandler> handler) {
    if (!is_valid_state(state_1))
        printf(debug_severity::ERROR, "Invalid current state number %d passed into function.",
               state_1);
    if (!is_valid_state(state_2))
        printf(debug_severity::ERROR, "Invalid current state number %d passed into function.",
               state_2);

    if (state_1 == state_2) {
        println(debug_severity::ERROR,
                "Should not be setting a transition handler "
                "between two of the same state.");
        return;
    }

    std::pair<unsigned int, unsigned int> state_pair(state_1, state_2);
    _transition_handlers.emplace(state_pair, handler);
}

template <size_t num_states>
bool StateMachine<num_states>::update_state_machine() {
    // Get current state and state handler
    const unsigned int cur_state = _state.get(std::shared_ptr<ControlTask<bool>>(this));
    if (!is_valid_state(cur_state))
        printf(debug_severity::ERROR, "Invalid state number %d for current state.", cur_state);

    std::shared_ptr<StateHandler> state_handler;
    try {
        state_handler = _state_handlers.at(cur_state);
    } catch (std::out_of_range &err) {
        printf(debug_console::ERROR, "No state handler specified for state %d!", cur_state);
        return false;
    }

    // Get next state and run transition handler
    if (!state_handler->only_execute_once || !state_handler->has_executed) {
        unsigned int next_state = state_handler->execute();
        state_handler->has_executed = true;

        if (!is_valid_state(next_state)) {
            printf(debug_console::ERROR, "Invalid state %d specified for next state.", next_state);
            return false;
        }

        std::pair<unsigned int, unsigned int> state_pair(cur_state, next_state);
        std::shared_ptr<TransitionHandler> transition_handler = _transition_handlers.at(state_pair);
        transition_handler->execute();
        // Set current state equal to next state
        return set_state(next_state);
    }
    return true;
}

template <size_t num_states>
bool StateMachine<num_states>::execute() {
    return update_state_machine();
}
