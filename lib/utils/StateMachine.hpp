#ifndef STATE_MACHINE_HPP_
#define STATE_MACHINE_HPP_

#include <map>
#include <string>
#include <type_traits>
#include <utility>
#include "ControlTask.hpp"
#include "SMStateField.hpp"
#include <StateField.hpp>
#include "StateMachineTasks.hpp"
#include "debug_console.hpp"

/**
 * @brief Singleton pattern used by derived classes
 * to limit construction of state machines.
 */
#define SINGLETON(classname)               \
    classname(const classname &) = delete; \
    classname &operator=(const classname &) = delete

/**
 * @brief A generic class for a state machine controller.
 */
template <size_t num_states>
class StateMachine : public ControlTask<bool> {
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
    void register_state_handler(unsigned int state, std::shared_ptr<StateHandler> handler);

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
     * @param name Name of state machine, as should be registered as a ControlTask.
     * @param state_name Name to set for the state machine state variable.
     * @param r Reference to a state field registry.
     * any.
     */
    StateMachine(const std::string &name, const std::string &state_name, StateFieldRegistry &r);

    SMStateField<num_states> _state;
    std::map<unsigned int, std::shared_ptr<StateHandler>> _state_handlers;
    std::map<std::pair<unsigned int, unsigned int>, std::shared_ptr<TransitionHandler>>
        _transition_handlers;

    SMStateSerializer<num_states> _state_serializer;
};

#include "StateMachine.inl"

#endif