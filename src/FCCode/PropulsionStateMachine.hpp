/**
 * @file PropulsionStateMachine.hpp
 * @author Tanishq Aggarwal (ta335@cornell.edu)
 * @date 2019-06-28
 */

#ifndef PROPULSION_STATE_MACHINE_HPP_
#define PROPULSION_STATE_MACHINE_HPP_

#include "StateMachine.hpp"

constexpr size_t num_prop_states = 2;  // TODO
constexpr size_t tank_pressure_sz = 2; // TODO
constexpr size_t prop_firing_sz = 2;   // TODO

template <>
class PropulsionStateMachine : public StateMachine<num_prop_states>
{
  private:
    // TODO add propulsion devices (SpikeAndHold).
  protected:
    ReadableStateField<temperature_t, temperature_t, SerializerConstants::temp_sz> tank_inner_temperature;
    ReadableStateField<temperature_t, temperature_t, SerializerConstants::temp_sz> tank_outer_temperature;
    ReadableStateField<float, float, tank_pressure_sz> tank_pressure;
    WritableStateField<bool, bool, SerializerConstants::bool_sz> intertank_valve;
    WritableStateField<gps_time_t, bool, SerializerConstants::gps_time_sz> firing_time;
    WritableStateField<f_vector_t, float, prop_firing_sz> firing_vector;
  public:
    PropulsionStateMachine(SMStateField<compressed_state_sz> &sv,
                           const std::string &name,
                           debug_console &dbg_console,
                           StateFieldRegistry &r);
};

#endif