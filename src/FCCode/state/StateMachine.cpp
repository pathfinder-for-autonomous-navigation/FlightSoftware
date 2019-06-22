#include "StateMachine.hpp"

StateHandler::StateHandler(std::string& name, 
                           debug_console& dbg) :
    ControlTask<unsigned int>(name, dbg) {
}

TransitionHandler::TransitionHandler(std::string& name, 
                                     debug_console& dbg) :
    ControlTask<void>(name, dbg) { }

StateMachine::StateMachine(StateMachineStateField& sv,
                           std::string& name,
                           debug_console& dbg_console) : 
    ControlTask(name, dbg), 
    _state(sv) {
    if (!sv.can_read(*this))
        dbg.printf(debug_severity::WARNING,
            "State machine %s does not have read access to its own state variable %s.", 
            name.c_str(),
            s.name().c_str());
    if (!sv.can_write(*this))
        dbg.printf(debug_severity::WARNING,
            "State machine %s does not have write access to its own state variable %s.", 
            name.c_str(),
            s.name().c_str());
}

void StateMachine::init(SMSField::state_t initial_state) {
    _check_enum(initial_state);
    set_state(initial_state);
}

void StateMachine::set_state(SMSField::state_t state) {
    _state.set(state);
}

void StateMachine::register_state_handler(SMSField::state_t state, StateHandler& handler) {
     _transition_handlers.emplace(state, &handler);
}

void StateMachine::register_transition_handler(SMSField::state_t state_1, 
                                               SMSField::state_t state_2, 
                                               TransitionHandler& handler) {
    std::pair<SMSField::state_t, SMSField::state_t> state_pair(state_1, state_2);
    _transition_handlers.emplace(state_pair, &handler);
}

void StateMachine::update_state_machine() {
    // Get current state and state handler
    SMSField::state_t cur_state = _state.get();
    StateHandler* state_handler = _state_handlers.at(cur_state);

    // Get next state and run pre-transition handler
    SMSField::state_t next_state = state_handler->execute();
    std::pair<SMSField::state_t, SMSField::state_t> state_pair(cur_state, next_state);
    TransitionHandler* transition_handler = _transition_handlers.at(state_pair);
    transition_handler->execute();
    
    // Set current state equal to next state
    set_state(next_state);
}

void StateMachine::execute() {
    update_state_machine();
}