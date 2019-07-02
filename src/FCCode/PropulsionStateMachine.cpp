/**
 * @file PropulsionStateMachine.cpp
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * @date 2019-06-28
 */

#include "PropulsionStateMachine.hpp"

std::array<std::string, num_prop_states> PropulsionStateMachine::state_names = {
    "safed",
    "idle",
    "waiting_for_pressurize",
    "pressurizing",
    "waiting_for_inject",
    "injecting"
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
                                                                        tank_inner_temperature("prop.inner_tank.temperature", r),
                                                                        tank_outer_temperature("prop.outer_tank.temperature", r),
                                                                        tank_pressure("prop.outer_tank.pressure", r),
                                                                        intertank_valve("prop.intertank_valve", r),
                                                                        firing_time("prop.firing.time", r),
                                                                        firing_vector("prop.firing.vector", r),
                                                                        temperature_serializer(),
                                                                        pressure_serializer(),
                                                                        intertank_valve_serializer(),
                                                                        firing_time_serializer(),
                                                                        firing_vector_serializer() {}

bool PropulsionStateMachine::init(unsigned int initial_state) {
    // Initializer serializers
    abort_if_init_fail(temperature_serializer.init(), (this->_dbg_console));
    abort_if_init_fail(pressure_serializer.init(0, max_serialization_pressure), (this->_dbg_console));
    abort_if_init_fail(intertank_valve_serializer.init(), (this->_dbg_console));
    abort_if_init_fail(firing_time_serializer.init(), (this->_dbg_console));
    abort_if_init_fail(firing_vector_serializer.init(0, max_impulse), (this->_dbg_console));

    // Initialize state variables
    abort_if_init_fail(tank_inner_temperature.init(&temperature_serializer, tank_inner_temp_fetcher, tank_inner_temp_sanity_check), (this->_dbg_console));
    abort_if_init_fail(tank_outer_temperature.init(&temperature_serializer, tank_outer_temp_fetcher, tank_outer_temp_sanity_check), (this->_dbg_console));
    abort_if_init_fail(tank_pressure.init(&pressure_serializer, pressure_fetcher, pressure_sanity_checker), _dbg_console);
    abort_if_init_fail(intertank_valve.init(&intertank_valve_serializer, StateFieldFunctions<bool>::null_fetcher, StateFieldFunctions<bool>::null_sanity_check), (this->_dbg_console));
    abort_if_init_fail(firing_time.init(&firing_time_serializer, StateFieldFunctions<gps_time_t>::null_fetcher, firing_time_sanity_check), (this->_dbg_console));
    abort_if_init_fail(firing_vector.init(&firing_vector_serializer, StateFieldFunctions<f_vector_t>::null_fetcher, firing_vector_sanity_check), (this->_dbg_console));
    abort_if_init_fail(StateMachine<num_prop_states>::init(state_names, initial_state), (this->_dbg_console));

    // Allow this state machine access to its own state variables
    _registry.add_writer(static_cast<Task&>(*this), tank_inner_temperature);
    _registry.add_writer(static_cast<Task&>(*this), tank_outer_temperature);
    _registry.add_writer(static_cast<Task&>(*this), tank_pressure);
    _registry.add_reader(static_cast<Task&>(*this), intertank_valve);
    _registry.add_reader(static_cast<Task&>(*this), firing_time);
    _registry.add_reader(static_cast<Task&>(*this), firing_vector);

    // Add state handlers
    for (unsigned int i = 0; i < static_cast<unsigned int>(state_t::num_states); i++)
    {
        const unsigned int i_cpy = i;
        const std::string state_handler_name = std::string("prop.state_handler.") + _state._state_names[i];
        register_state_handler(i, new PropulsionStateHandler<(state_t)i_cpy>(state_handler_name));
    }

    // Add state transition handlers
    for (unsigned int i = 0; i < static_cast<unsigned int>(state_transition_t::num_transitions); i++)
    {
        const unsigned int i_cpy = i;
        const std::string transition_handler_name = std::string("prop.transition_handler.") + _state._state_names[i];
        register_state_handler(i, new PropulsionTransitionHandler<(state_transition_t)i_cpy>(transition_handler_name));
    }

    // Give the state and transition handlers access to their required variables
    // TODO

    return true;
}