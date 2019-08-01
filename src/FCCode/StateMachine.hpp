#ifndef STATE_MACHINE_HPP_
#define STATE_MACHINE_HPP_

#include "ControlTask.hpp"
#include "SMStateField.hpp"
#include "StateField.hpp"
#include "StateMachineTasks.hpp"
#include "debug_console.hpp"
#include <map>
#include <string>
#include <type_traits>
#include <utility>

/**
 * @brief Singleton pattern used by derived classes
 * to limit construction of state machines.
 */
#define SINGLETON(classname)                                                   \
  classname(const classname &) = delete;                                       \
  classname &operator=(const classname &) = delete

/**
 * @brief A generic class for a state machine controller.
 */
template <size_t num_states>
class StateMachine : public StateFieldRegistryReader<bool> {
public:
  /**
   * @brief Checks if provided state is a valid state. The
   * valid states are defined in the protected state variable field.
   *
   * @param state The state to check
   * @return Whether or not the state is valid
   */
  static bool is_valid_state(unsigned int state);

  /**
   * @brief Initialize state to the specified state.
   *
   * @param state_names Names to assign the various states.
   * @param initial_state State to set the state machine in.
   * @return True if succeeded, false if state variable is not initialized.
   */
  virtual bool init(const std::array<std::string, num_states> &state_names,
                    unsigned int initial_state);

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
  void register_state_handler(unsigned int state,
                              std::shared_ptr<StateHandler> handler);

  /**
   * @brief Add a ControlTask handler that executes prior to a transition
   * from state 1 to state 2. The state transition happens within
   * update_state_machine, after the state handler has already been called.
   *
   * @param state_1
   * @param state_2
   * @param handler
   * @return True if succeeded, false if state variable is not initialized.
   */
  void register_transition_handler(unsigned int state_1, unsigned int state_2,
                                   std::shared_ptr<TransitionHandler> handler);

  /**
   * @brief Run a full update cycle on the state machine, which means running
   * the state handler for the current state, getting the value of the next
   * state, running the pre-transition handler and setting the next state.
   *
   * @return True if succeeded, false if state variable is not initialized.
   */
  bool update_state_machine();
  bool execute() override;

protected:
  /**
   * @brief Construct a new State Machine object
   *
   * This constructor is protected so that a basic StateMachine cannot be
   * instantiated; only derived classes can, and they must specify an explicit
   * public constructor.
   *
   * @param sv Variable that contains the state upon which this state machine
   * acts.
   * @param dbg_console Console for outputting state machine error messages, if
   * any.
   */
  StateMachine(const std::string &name, const std::string &state_name,
               StateFieldRegistry &r);

  SMStateField<num_states> _state;
  std::map<unsigned int, std::shared_ptr<StateHandler>> _state_handlers;
  std::map<std::pair<unsigned int, unsigned int>,
           std::shared_ptr<TransitionHandler>>
      _transition_handlers;

private:
  SMStateSerializer<num_states> _state_serializer;
};

template <size_t num_states>
StateMachine<num_states>::StateMachine(const std::string &name,
                                       const std::string &state_name,
                                       StateFieldRegistry &r)
    : StateFieldRegistryReader(name, r), _state(state_name, r),
      _state_handlers(), _transition_handlers(), _state_serializer() {}

template <size_t num_states>
bool StateMachine<num_states>::is_valid_state(unsigned int state) {
  return state < static_cast<unsigned int>(num_states);
}

template <size_t num_states>
bool StateMachine<num_states>::init(
    const std::array<std::string, num_states> &state_names,
    unsigned int initial_state) {
  _state_serializer.init();

  std::shared_ptr<SMStateSerializer<num_states>> serializer_ptr(
      &_state_serializer);
  _state.init(state_names,
              serializer_ptr); // We use the default sanity checker, because
                               // we already check this value via
                               // is_valid_state() assertions.

  // TODO give this state machine access to its own state variable.

  if (!is_valid_state(initial_state)) {
    printf(debug_severity::ERROR,
           "Invalid state number %d passed into function.", initial_state);
    return false;
  }

  return set_state(initial_state);
}

template <size_t num_states>
bool StateMachine<num_states>::set_state(unsigned int state) {
  if (!is_valid_state(state)) {
    printf(debug_severity::ERROR,
           "Invalid state number %d passed into function.", state);
    return false;
  }

  if (!_state.set(std::shared_ptr<Task>(this), state))
    return false;
  std::shared_ptr<StateHandler> state_handler = _state_handlers.at(state);
  state_handler->has_executed = false;

  return true;
}

template <size_t num_states>
void StateMachine<num_states>::register_state_handler(
    unsigned int state, std::shared_ptr<StateHandler> handler) {
  if (!is_valid_state(state))
    printf(debug_severity::ERROR,
           "Invalid state number %d passed into function.", state);

  _state_handlers.emplace(state, handler);
}

template <size_t num_states>
void StateMachine<num_states>::register_transition_handler(
    unsigned int state_1, unsigned int state_2,
    std::shared_ptr<TransitionHandler> handler) {
  if (!is_valid_state(state_1))
    printf(debug_severity::ERROR,
           "Invalid current state number %d passed into function.", state_1);
  if (!is_valid_state(state_2))
    printf(debug_severity::ERROR,
           "Invalid current state number %d passed into function.", state_2);

  if (state_1 == state_2) {
    println(debug_severity::ERROR, "Should not be setting a transition handler "
                                   "between two of the same state.");
    return;
  }

  std::pair<unsigned int, unsigned int> state_pair(state_1, state_2);
  _transition_handlers.emplace(state_pair, handler);
}

template <size_t num_states>
bool StateMachine<num_states>::update_state_machine() {
  // Get current state and state handler
  const unsigned int cur_state = _state.get(std::shared_ptr<Task>(this));
  if (!is_valid_state(cur_state))
    printf(debug_severity::ERROR, "Invalid state number %d for current state.",
           cur_state);

  std::shared_ptr<StateHandler> state_handler;
  try {
    state_handler = _state_handlers.at(cur_state);
  } catch (std::out_of_range &err) {
    printf(debug_console::ERROR, "No state handler specified for state %d!",
           cur_state);
    return false;
  }

  // Get next state and run transition handler
  if (!state_handler->only_execute_once || !state_handler->has_executed) {
    unsigned int next_state = state_handler->execute();
    state_handler->has_executed = true;

    if (!is_valid_state(next_state)) {
      printf(debug_console::ERROR, "Invalid state %d specified for next state.",
             next_state);
      return false;
    }

    std::pair<unsigned int, unsigned int> state_pair(cur_state, next_state);
    std::shared_ptr<TransitionHandler> transition_handler =
        _transition_handlers.at(state_pair);
    transition_handler->execute();
    // Set current state equal to next state
    return set_state(next_state);
  }
  return true;
}

template <size_t num_states> bool StateMachine<num_states>::execute() {
  return update_state_machine();
}

#endif