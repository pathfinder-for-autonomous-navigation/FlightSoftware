#include "propulsion_tasks.hpp"
#include "../../state/fault_state_holder.hpp"

using State::Hardware::spike_and_hold_device_lock;
using namespace Constants::Propulsion;
using Devices::spike_and_hold;
using State::Propulsion::propulsion_state_lock;

thread_t* PropulsionTasks::pressurizing_thread;

THD_WORKING_AREA(PropulsionTasks::pressurizing_thread_wa, 1024);

THD_FUNCTION(PropulsionTasks::pressurizing_fn, args) {
    float tank_pressure = State::read(State::Propulsion::tank_pressure, propulsion_state_lock);

    systime_t t0 = chVTGetSystemTimeX();
    gps_time_t firing_time = State::read(State::Propulsion::firing_data.time, propulsion_state_lock); 
    unsigned int dt = (unsigned int) (firing_time - State::GNC::get_current_time());
    systime_t tf = t0 + MS2ST(dt);

    for(int i = 0; i < 20; i++) {
        std::array<unsigned int, 6> firings;
        unsigned char preferred_valve = State::read(State::Propulsion::intertank_firing_valve, propulsion_state_lock);
        unsigned int valve_vent_time = Constants::read(VALVE_VENT_TIME);
        firings[preferred_valve] = valve_vent_time;
        
        if (State::Hardware::check_is_functional(spike_and_hold)) {
            chMtxLock(&spike_and_hold_device_lock);
                spike_and_hold.execute_schedule(firings);
            chMtxUnlock(&spike_and_hold_device_lock);
        }
        else break;

        chThdSleepMilliseconds(WAIT_BETWEEN_PRESSURIZATIONS);

        State::write(State::Propulsion::tank_pressure, Devices::pressure_sensor.get(), propulsion_state_lock);
        tank_pressure = State::read(State::Propulsion::tank_pressure, propulsion_state_lock);
        if (tank_pressure >= PRE_FIRING_OUTER_TANK_PRESSURE) break;
    }
    
    if (tank_pressure < PRE_FIRING_OUTER_TANK_PRESSURE) {
        // Recommend satellite safe hold.
        State::write(FaultState::Propulsion::cannot_pressurize_outer_tank, 
            true, FaultState::Propulsion::propulsion_faults_state_lock);
    }

    chThdSleepUntil(tf);
    State::write(State::Propulsion::propulsion_state, 
                 State::Propulsion::PropulsionState::FIRING, 
                 State::Propulsion::propulsion_state_lock);
    chThdExit((msg_t) 0);
}