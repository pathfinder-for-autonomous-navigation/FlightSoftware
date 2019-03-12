#include "propulsion_tasks.hpp"
#include "../../state/fault_state_holder.hpp"

using State::Hardware::spike_and_hold_device_lock;
using namespace Constants::Propulsion;
using Devices::spike_and_hold;
using State::Propulsion::propulsion_state_lock;

thread_t* PropulsionTasks::pressurizing_thread;

THD_WORKING_AREA(PropulsionTasks::pressurizing_thread_wa, 1024);

THD_FUNCTION(PropulsionTasks::pressurizing_fn, args) {
    float tank_pressure;

    for(int i = 0; i < 20; i++) {
        std::array<unsigned int, 6> firings;
        unsigned char preferred_valve = State::read(State::Propulsion::intertank_firing_valve, propulsion_state_lock);
        firings[preferred_valve] = VALVE_VENT_TIME;
        chMtxLock(&spike_and_hold_device_lock);
            if (State::Hardware::can_get_data(Devices::spike_and_hold)) {
                spike_and_hold.execute_schedule(firings);
            }
        chMtxUnlock(&spike_and_hold_device_lock);

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
}