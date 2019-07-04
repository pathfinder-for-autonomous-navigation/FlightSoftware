/**
 * @file PropulsionStateMachine.cpp
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * @date 2019-06-28
 */

#include "PropulsionStateMachine.hpp"

std::array<std::string, 
           static_cast<unsigned int>(PropulsionStateMachine::state_t::num_states)> 
           PropulsionStateMachine::state_names = {
    "safed",
    "idle",
    "waiting_for_pressurize",
    "pressurizing",
    "waiting_for_inject",
    "injecting"
};

std::array<std::string, 
           static_cast<unsigned int>(PropulsionStateMachine::state_transition_t::num_transitions)> 
           PropulsionStateMachine::transition_names = {
    "safed_to_idle",

    "idle_to_safed",
    "idle_to_wait_for_pressurize",

    "wait_for_pressurize_to_pressurizing",
    "wait_for_pressurize_to_idle",
    "wait_for_pressurize_to_safed",

    "pressurizing_to_wait_for_inject",
    "pressurizing_to_idle",
    "pressurizing_to_safed",

    "wait_for_inject_to_injecting",
    "wait_for_inject_to_idle",
    "wait_for_inject_to_safed",

    "injecting_to_idle",
};

float PropulsionStateMachine::pressure_fetcher() {
    return 0.0f; // TODO
}

temperature_t PropulsionStateMachine::tank_inner_temp_fetcher() {
    return 10; // TODO
}

temperature_t PropulsionStateMachine::tank_outer_temp_fetcher() {
    return 10; // TODO
}

bool PropulsionStateMachine::pressure_sanity_check(const float& pressure) { return (pressure > 0) && (pressure < max_sanity_pressure); }

bool PropulsionStateMachine::tank_inner_temp_sanity_check(const temperature_t& temp) {
    return (temp > 0) && (temp < max_sanity_inner_tank_temperature);
}

bool PropulsionStateMachine::tank_outer_temp_sanity_check(const temperature_t& temp) {
    return (temp > 0) && (temp < max_sanity_outer_tank_temperature);
}

bool PropulsionStateMachine::firing_time_sanity_check(const gps_time_t& time) {
    // TODO
}

bool PropulsionStateMachine::firing_vector_sanity_check(const f_vector_t& vec) {
    // TODO
}

PropulsionStateMachine::PropulsionStateMachine(StateFieldRegistry &r) : StateMachine<num_prop_states>("prop.sm", "prop.sm_state", r),
                                                                        temperature_serializer(),
                                                                        pressure_serializer(),
                                                                        intertank_valve_serializer(),
                                                                        firing_time_serializer(),
                                                                        firing_vector_serializer(),
                                                                        tank_inner_temperature("prop.inner_tank.temperature", r),
                                                                        tank_outer_temperature("prop.outer_tank.temperature", r),
                                                                        tank_pressure("prop.outer_tank.pressure", r),
                                                                        intertank_valve("prop.intertank_valve", r),
                                                                        firing_time("prop.firing.time", r),
                                                                        firing_vector("prop.firing.vector", r) {}

// Shorten static_cast expression via a macro
#define STATE_INT(state) static_cast<unsigned int>(state_t::state)

bool PropulsionStateMachine::init(unsigned int initial_state) {
    // Initializer serializers
    abort_if_init_fail(temperature_serializer.init());
    abort_if_init_fail(pressure_serializer.init(0, max_serialization_pressure));
    abort_if_init_fail(intertank_valve_serializer.init());
    abort_if_init_fail(firing_time_serializer.init());
    abort_if_init_fail(firing_vector_serializer.init(0, max_impulse));

    // Initialize state variables
    abort_if_init_fail(tank_inner_temperature.init(&temperature_serializer, tank_inner_temp_fetcher, pressure_sanity_checker));
    abort_if_init_fail(tank_outer_temperature.init(&temperature_serializer, tank_outer_temp_fetcher, tank_outer_temp_sanity_checker));
    abort_if_init_fail(tank_pressure.init(&pressure_serializer, pressure_fetcher, pressure_sanity_checker));
    abort_if_init_fail(intertank_valve.init(&intertank_valve_serializer, StateFieldFunctions<bool>::null_fetcher, StateFieldFunctions<bool>::null_sanity_check));
    abort_if_init_fail(firing_time.init(&firing_time_serializer, StateFieldFunctions<gps_time_t>::null_fetcher, firing_time_sanity_checker));
    abort_if_init_fail(firing_vector.init(&firing_vector_serializer, StateFieldFunctions<f_vector_t>::null_fetcher, firing_vector_sanity_checker));
    abort_if_init_fail(StateMachine<num_prop_states>::init(state_names, static_cast<unsigned int>(state_t::safed)));

    // Allow this state machine access to its own state variables
    _registry.add_writer(static_cast<Task&>(*this), static_cast<DataField&>(tank_inner_temperature));
    _registry.add_writer(static_cast<Task&>(*this), static_cast<DataField&>(tank_outer_temperature));
    _registry.add_writer(static_cast<Task&>(*this), static_cast<DataField&>(tank_pressure));
    _registry.add_reader(static_cast<Task&>(*this), static_cast<DataField&>(intertank_valve));
    _registry.add_reader(static_cast<Task&>(*this), static_cast<DataField&>(firing_time));
    _registry.add_reader(static_cast<Task&>(*this), static_cast<DataField&>(firing_vector));

    // Add state handlers. At the end, make sure every state has a registered handler, and no handlers exist
    // for undefined states.
    #define REGISTER_STATE_HANDLER(name)                                                        \
        register_state_handler(static_cast<unsigned int>(state_t::name),                        \
        *new PropulsionStateHandler<state_t::name>("prop.state_handler." #name, _registry)); 

    REGISTER_STATE_HANDLER(safed);
    REGISTER_STATE_HANDLER(idle);
    REGISTER_STATE_HANDLER(wait_for_pressurize);
    REGISTER_STATE_HANDLER(pressurizing);
    REGISTER_STATE_HANDLER(wait_for_inject);
    REGISTER_STATE_HANDLER(injecting);

    #undef REGISTER_STATE_HANDLER

    abort_if_not(_state_handlers.size() == state_names.size());
    abort_if_not(_state._state_names == state_names);
    for (unsigned int i = 0; i < static_cast<unsigned int>(state_t::num_states); i++)
    {
        const std::string state_handler_name = std::string("prop.state_handler.") + _state._state_names[i];
        abort_if_not(_state_handlers.find(i).second->name() != state_handler_name);
    }

    // Add state transition handlers. At the end, make sure every transition is accounted for (i.e. every required transition has
    // been registered, and no invalid transitions have been registered.)
    #define REGISTER_TRANSITION_HANDLER(state1, state2, transition)                                                           \
        register_transition_handler(static_cast<unsigned int>(state_t::name1),                                                \
                                    static_cast<unsigned int>(state_t::name2),                                                \
        *new PropulsionTransitionHandler<state_transition_t::transition>("prop.transition_handler." #transition, _registry)); 

    REGISTER_TRANSITION_HANDLER(safed, idle, safed_to_idle);
    REGISTER_TRANSITION_HANDLER(idle, safed, idle_to_safed);
    REGISTER_TRANSITION_HANDLER(idle, wait_for_pressurize, idle_to_wait_for_pressurize);
    REGISTER_TRANSITION_HANDLER(wait_for_pressurize, pressurizing, wait_for_pressurize_to_pressurizing);
    REGISTER_TRANSITION_HANDLER(wait_for_pressurize, idle, wait_for_pressurize_to_idle);
    REGISTER_TRANSITION_HANDLER(wait_for_pressurize, safed, wait_for_pressurize_to_safed);
    REGISTER_TRANSITION_HANDLER(pressurizing, wait_for_inject, pressurizing_to_wait_for_inject);
    REGISTER_TRANSITION_HANDLER(pressurizing, idle, pressurizing_to_idle);
    REGISTER_TRANSITION_HANDLER(pressurizing, safed, pressurizing_to_safed);
    REGISTER_TRANSITION_HANDLER(wait_for_inject, injecting, wait_for_inject_to_injecting);
    REGISTER_TRANSITION_HANDLER(wait_for_inject, idle, wait_for_inject_to_idle);
    REGISTER_TRANSITION_HANDLER(wait_for_inject, safed, wait_for_inject_to_safed);
    REGISTER_TRANSITION_HANDLER(injecting, idle, injecting_to_idle);

    #undef REGISTER_TRANSITION_HANDLER

    abort_if_not(_transition_handlers.size() == transition_names.size());
    for (unsigned int i = 0; i < static_cast<unsigned int>(state_transition_t::num_transitions); i++)
    {
        const std::string transition_handler_name = std::string("prop.transition_handler.") + transition_names[i];
        // TODO find transition and execute
    }

    // Give the state and transition handlers access to their required variables
    // TODO

    return true;
}
