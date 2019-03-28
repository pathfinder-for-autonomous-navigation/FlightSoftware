#include "../../state/fault_state_holder.hpp"
#include "../../state/state_holder.hpp"
#include "propulsion_tasks.hpp"
#include "../constants.hpp"

using Devices::pressure_sensor;
using Devices::temp_sensor_inner;
using Devices::temp_sensor_outer;
using State::Propulsion::PropulsionState;
using State::Propulsion::propulsion_state_lock;

int PropulsionTasks::can_fire_manuever() {
    if(State::Hardware::check_is_functional(&pressure_sensor()))
        State::write(State::Propulsion::tank_pressure, pressure_sensor().get(), propulsion_state_lock);
    if(State::Hardware::check_is_functional(&temp_sensor_inner()))
        State::write(State::Propulsion::tank_inner_temperature, temp_sensor_inner().get(), propulsion_state_lock);
    if(State::Hardware::check_is_functional(&temp_sensor_outer()))
        State::write(State::Propulsion::tank_outer_temperature, temp_sensor_outer().get(), propulsion_state_lock);

    bool is_outer_tank_pressure_too_high = State::read(State::Propulsion::tank_pressure, propulsion_state_lock) >= 48
                                            && State::Hardware::check_is_functional(&pressure_sensor());
    bool is_inner_tank_temperature_too_high = State::read(State::Propulsion::tank_inner_temperature, propulsion_state_lock) >= 100 
                                            && State::Hardware::check_is_functional(&temp_sensor_inner());
    bool is_outer_tank_temperature_too_high = State::read(State::Propulsion::tank_outer_temperature, propulsion_state_lock) >= 48
                                            && State::Hardware::check_is_functional(&temp_sensor_outer());

    if (is_inner_tank_temperature_too_high || is_outer_tank_temperature_too_high || is_outer_tank_pressure_too_high) {
        State::write(FaultState::Propulsion::overpressure_event,
                     State::GNC::get_current_time(),
                     FaultState::Propulsion::propulsion_faults_state_lock);

        if (is_inner_tank_temperature_too_high) {
            State::write(FaultState::Propulsion::overpressure_event_id,
                         FaultState::Propulsion::OVERPRESSURE_EVENT::INNER_TANK_TEMPERATURE,
                         FaultState::Propulsion::propulsion_faults_state_lock);
        }
        else if (is_outer_tank_temperature_too_high) {
            State::write(FaultState::Propulsion::overpressure_event_id,
                         FaultState::Propulsion::OVERPRESSURE_EVENT::OUTER_TANK_TEMPERATURE,
                         FaultState::Propulsion::propulsion_faults_state_lock);
        }
        else if (is_outer_tank_pressure_too_high) {
            State::write(FaultState::Propulsion::overpressure_event_id,
                         FaultState::Propulsion::OVERPRESSURE_EVENT::OUTER_TANK_PRESSURE,
                         FaultState::Propulsion::propulsion_faults_state_lock);
        }
        
        return -1;
    }

    gps_time_t firing_time = State::read(State::Propulsion::firing_data.time, propulsion_state_lock);

    gps_time_t current_time = State::GNC::get_current_time();
    if (current_time > firing_time - Constants::Propulsion::thruster_preparation_time()) {
        // We cannot execute this firing, since the planned time of the 
        // firing (and its preparation) is less than the current time!
        return 0;
    }

    bool is_standby = State::read(State::Master::pan_state, State::Master::master_state_lock) == State::Master::PANState::STANDBY;
    bool is_nighttime = !State::read(State::ADCS::is_sun_vector_determination_working, State::ADCS::adcs_state_lock);
    bool has_firing_happened_in_nighttime = State::read(State::GNC::has_firing_happened_in_nighttime, State::ADCS::adcs_state_lock);
    if (is_nighttime && has_firing_happened_in_nighttime && !is_standby) {
        // We cannot execute the firing since we've already done one at night! Also,
        // we're not in standby mode, so we can't force a firing.
        return 0;
    }
    else if (!is_nighttime) {
        // We're now in daylight! So we can set the "has firing happened in nighttime" flag to false
        // since it'll be a while before we return to nighttime.
        has_firing_happened_in_nighttime = false;
    }
    return 1;
}