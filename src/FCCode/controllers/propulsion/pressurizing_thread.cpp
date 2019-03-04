#include "propulsion_tasks.hpp"

using State::Hardware::spike_and_hold_lock;
using namespace Constants::Propulsion;
using Devices::spike_and_hold;

thread_t* PropulsionTasks::pressurizing_thread;

THD_WORKING_AREA(PropulsionTasks::pressurizing_thread_wa, 1024);

THD_FUNCTION(PropulsionTasks::pressurizing_fn, args) {
    chMtxLock(&spike_and_hold_lock);
    // Pressurize tank
    chMtxUnlock(&spike_and_hold_lock);
    // TODO count pressurizations, check if pressure is starting to match the intended pressure
    // If not, send satellite to safe hold.
}