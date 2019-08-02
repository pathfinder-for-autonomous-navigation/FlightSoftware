/**
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * @brief Contains definitions for state machine handlers for
 * propulsion state machine.
 */

#include "PropulsionStateMachine.hpp"

// State Handler specializations
template <>
class PropulsionStateMachine::PropulsionStateHandler<PropulsionStateMachine::state_t::idle>
    : public StateHandler {
   public:
    using StateHandler::StateHandler;
    unsigned int execute() override { return 0; }
};

template <>
class PropulsionStateMachine::PropulsionStateHandler<PropulsionStateMachine::state_t::safed>
    : public StateHandler {
   public:
    using StateHandler::StateHandler;
    unsigned int execute() override { return 0; }
};

template <>
class PropulsionStateMachine::PropulsionStateHandler<
    PropulsionStateMachine::state_t::wait_for_pressurize> : public StateHandler {
   public:
    using StateHandler::StateHandler;
    unsigned int execute() override { return 0; }
};

template <>
class PropulsionStateMachine::PropulsionStateHandler<PropulsionStateMachine::state_t::pressurizing>
    : public StateHandler {
   public:
    using StateHandler::StateHandler;
    unsigned int execute() override { return 0; }
};

template <>
class PropulsionStateMachine::PropulsionStateHandler<
    PropulsionStateMachine::state_t::wait_for_inject> : public StateHandler {
   public:
    using StateHandler::StateHandler;
    unsigned int execute() override { return 0; }
};

template <>
class PropulsionStateMachine::PropulsionStateHandler<PropulsionStateMachine::state_t::injecting>
    : public StateHandler {
   public:
    using StateHandler::StateHandler;
    unsigned int execute() override { return 0; }
};

// Transition handler specializations
template <>
class PropulsionStateMachine::PropulsionTransitionHandler<
    PropulsionStateMachine::state_transition_t::safed_to_idle> : public TransitionHandler {
   public:
    using TransitionHandler::TransitionHandler;
    void execute() override {}
};

template <>
class PropulsionStateMachine::PropulsionTransitionHandler<
    PropulsionStateMachine::state_transition_t::idle_to_safed> : public TransitionHandler {
   public:
    using TransitionHandler::TransitionHandler;
    void execute() override {}
};

template <>
class PropulsionStateMachine::PropulsionTransitionHandler<
    PropulsionStateMachine::state_transition_t::idle_to_wait_for_pressurize>
    : public TransitionHandler {
   public:
    using TransitionHandler::TransitionHandler;
    void execute() override {}
};

template <>
class PropulsionStateMachine::PropulsionTransitionHandler<
    PropulsionStateMachine::state_transition_t::wait_for_pressurize_to_pressurizing>
    : public TransitionHandler {
   public:
    using TransitionHandler::TransitionHandler;
    void execute() override {}
};

template <>
class PropulsionStateMachine::PropulsionTransitionHandler<
    PropulsionStateMachine::state_transition_t::wait_for_pressurize_to_idle>
    : public TransitionHandler {
   public:
    using TransitionHandler::TransitionHandler;
    void execute() override {}
};

template <>
class PropulsionStateMachine::PropulsionTransitionHandler<
    PropulsionStateMachine::state_transition_t::wait_for_pressurize_to_safed>
    : public TransitionHandler {
   public:
    using TransitionHandler::TransitionHandler;
    void execute() override {}
};

template <>
class PropulsionStateMachine::PropulsionTransitionHandler<
    PropulsionStateMachine::state_transition_t::pressurizing_to_wait_for_inject>
    : public TransitionHandler {
   public:
    using TransitionHandler::TransitionHandler;
    void execute() override {}
};

template <>
class PropulsionStateMachine::PropulsionTransitionHandler<
    PropulsionStateMachine::state_transition_t::pressurizing_to_idle> : public TransitionHandler {
   public:
    using TransitionHandler::TransitionHandler;
    void execute() override {}
};

template <>
class PropulsionStateMachine::PropulsionTransitionHandler<
    PropulsionStateMachine::state_transition_t::pressurizing_to_safed> : public TransitionHandler {
   public:
    using TransitionHandler::TransitionHandler;
    void execute() override {}
};

template <>
class PropulsionStateMachine::PropulsionTransitionHandler<
    PropulsionStateMachine::state_transition_t::wait_for_inject_to_injecting>
    : public TransitionHandler {
   public:
    using TransitionHandler::TransitionHandler;
    void execute() override {}
};

template <>
class PropulsionStateMachine::PropulsionTransitionHandler<
    PropulsionStateMachine::state_transition_t::wait_for_inject_to_idle>
    : public TransitionHandler {
   public:
    using TransitionHandler::TransitionHandler;
    void execute() override {}
};

template <>
class PropulsionStateMachine::PropulsionTransitionHandler<
    PropulsionStateMachine::state_transition_t::wait_for_inject_to_safed>
    : public TransitionHandler {
   public:
    using TransitionHandler::TransitionHandler;
    void execute() override {}
};

template <>
class PropulsionStateMachine::PropulsionTransitionHandler<
    PropulsionStateMachine::state_transition_t::injecting_to_idle> : public TransitionHandler {
   public:
    using TransitionHandler::TransitionHandler;
    void execute() override {}
};