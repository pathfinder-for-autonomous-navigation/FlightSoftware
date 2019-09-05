#ifndef STATE_MACHINE_TASKS_HPP_
#define STATE_MACHINE_TASKS_HPP_

#include "ControlTask.hpp"
#include "StateField.hpp"

/**
 * @brief Represents a control ControlTaskBase specifically designed for handling
 * the actions that should be run during a state.
 *
 */
class StateHandler : public ControlTask<unsigned int> {
   public:
    /**
     * @brief If true, the state handler is run only once by the state
     * machine, when the machine enters the state. Otherwise, the state handler
     * runs execute() on every dispatch of the state machine.
     *
     */
    bool only_execute_once;

    /**
     * @brief Tracks whether or not the execute() function has already been run
     * once.
     *
     */
    bool has_executed = false;

    /**
     * @brief Construct a new State Handler object
     *
     * @param only_once If this is true, the state handler is run only once by the
     * state machine, when the machine enters the state. Otherwise, the state
     * handler runs execute() on every dispatch of the state machine.
     */
    StateHandler(const std::string &name, StateFieldRegistry &r, bool only_once = false)
        : ControlTask<unsigned int>(name, r), only_execute_once(only_once) {}

    virtual unsigned int execute() = 0;
};

/**
 * @brief Represents a control ControlTaskBase specifically designed for handling
 * the actions that should happen during a transition between two states.
 *
 */
class TransitionHandler : public ControlTask<void> {
   public:
    using ControlTask<void>::ControlTask;
    virtual void execute() = 0;
};

#endif