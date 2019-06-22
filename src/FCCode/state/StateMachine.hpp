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

class StateHandler : public ControlTask<unsigned int> {
  public:
    StateHandler(std::string& name,
                 debug_console& dbg);
    virtual unsigned int execute() = 0;
};

class TransitionHandler : public ControlTask<void> {
  public:
    TransitionHandler(std::string& name, 
                      debug_console& dbg);
    virtual void execute() = 0;
};

/**
 * @brief A generic class for a state machine controller.
 */
class StateMachine : public ControlTask<void> {
  public:
    /**
     * @brief Construct a new State Machine object
     * 
     * @param sv Variable that contains the state upon which this state machine acts.
     * @param dbg_console Console for outputting state machine error messages, if any.
     */
    StateMachine(SMStateFieldBase& sv,
                 std::string& name,
                 debug_console& dbg_console);

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
    SMStateFieldBase& _state;
    std::map<unsigned int, StateHandler*> _state_handlers;
    std::map<std::pair<unsigned int, unsigned int>, TransitionHandler*> _transition_handlers;
};

#endif