/**
 * @file PropulsionStateMachine.cpp
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * @date 2019-06-28
 */

#include "PropulsionStateMachine.hpp"

PropulsionStateMachine::PropulsionStateMachine(SMStateField<num_prop_states> &sv,
                                               const std::string &name,
                                               debug_console &dbg_console,
                                               StateFieldRegistry &r) : StateMachine<num_prop_states>(sv, name, dbg_console, r),
                                                                        tank_inner_temperature("prop.inner_tank.temperature", dbg_console, r),
                                                                        tank_outer_temperature("prop.outer_tank.temperature", dbg_console, r),
                                                                        tank_pressure("prop.outer_tank.pressure", dbg_console, r),
                                                                        intertank_valve("prop.intertank_valve", dbg_console, r),
                                                                        firing_time("prop.firing.time", dbg_console, r),
                                                                        firing_vector("prop.firing.vector", dbg_console, r) {}

bool PropulsionStateMachine::init(unsigned int initial_state) {
    abort_if_not_fail(
        tank_inner_temperature.init(serializer, fetcher, checker), _dbg_console));
    abort_if_not_fail(
        tank_outer_temperature.init(serializer, fetcher, checker));
    abort_if_not_fail(
        tank_pressure.init(serializer, fetcher, checker));
    abort_if_not_fail(
        intertank_valve.init(serializer, fetcher, checker));
    abort_if_not_fail(
        firing_time.init(serializer, fetcher, checker));
    abort_if_not_fail(
        firing_vector.init(serializer, fetcher, checker));
    abort_if_not_fail(
        StateMachine<num_prop_states>::init(initial_state));
    return true;
}