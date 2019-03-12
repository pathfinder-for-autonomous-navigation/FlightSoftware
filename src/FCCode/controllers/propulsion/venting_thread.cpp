#include "propulsion_tasks.hpp"

using State::Hardware::spike_and_hold_device_lock;
using namespace Constants::Propulsion;
using Devices::spike_and_hold;

thread_t* PropulsionTasks::venting_thread;
THD_WORKING_AREA(PropulsionTasks::venting_thread_wa, 1024);
THD_FUNCTION(PropulsionTasks::venting_fn, args) {
    debug_println("Tank temperature or pressures are too high. Venting to reduce vapor pressure.");
    // TODO notify ground that overpressure event happened.
    for(int i = 0; i < 10; i++) {
        chMtxLock(&spike_and_hold_device_lock);
            if (State::Hardware::can_get_data(Devices::spike_and_hold)) {
                spike_and_hold.execute_schedule({VALVE_VENT_TIME, VALVE_VENT_TIME, 0, 0, 0, 0});
            }
        chMtxUnlock(&spike_and_hold_device_lock);
        chThdSleepMilliseconds(VALVE_WAIT_TIME);
    }
    chThdExit((msg_t) 0);
}