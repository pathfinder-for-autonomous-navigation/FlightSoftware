#include "propulsion_tasks.hpp"
#include "../../state/fault_state_holder.hpp"

using State::Hardware::spike_and_hold_lock;
using namespace Constants::Propulsion;
using Devices::spike_and_hold;
using State::Propulsion::propulsion_state_lock;

thread_t* PropulsionTasks::pressurizing_thread;

THD_WORKING_AREA(PropulsionTasks::pressurizing_thread_wa, 1024);

THD_FUNCTION(PropulsionTasks::pressurizing_fn, args) {
    float tank_pressure;

    for(int i = 0; i < 20; i++) {
        std::array<unsigned int, 6> firings;
        rwMtxRLock(&propulsion_state_lock);
            firings[State::Propulsion::intertank_firing_valve] = VALVE_VENT_TIME;
        rwMtxRLock(&propulsion_state_lock);
        chMtxLock(&spike_and_hold_lock);
            spike_and_hold.execute_schedule(firings);
        chMtxUnlock(&spike_and_hold_lock);

        chThdSleepMilliseconds(WAIT_BETWEEN_PRESSURIZATIONS);

        rwMtxWLock(&propulsion_state_lock);
            State::Propulsion::tank_pressure = Devices::pressure_sensor.get();
            tank_pressure = State::Propulsion::tank_pressure;
        rwMtxWUnlock(&propulsion_state_lock);
        if (tank_pressure >= PRE_FIRING_OUTER_TANK_PRESSURE) break;
    }
    
    if (tank_pressure < PRE_FIRING_OUTER_TANK_PRESSURE) {
        // Recommend satellite safe hold.
        rwMtxWLock(&FaultState::Propulsion::propulsion_fault_state_lock);
            FaultState::Propulsion::cannot_pressurize_outer_tank = true;
        rwMtxWUnlock(&FaultState::Propulsion::propulsion_fault_state_lock);
    }
}