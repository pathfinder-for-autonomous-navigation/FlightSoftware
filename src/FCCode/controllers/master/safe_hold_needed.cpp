#include "master_helpers.hpp"

using State::Master::master_state_lock;

unsigned short int Master::safe_hold_needed() {
    bool autoexited_safe_hold = State::read(State::Master::autoexited_safe_hold, master_state_lock);
    if (autoexited_safe_hold) return 0; // Don't set up safehold in this case.

    unsigned short int reason = 0; // No reason

    // Checks all flags in Master State Holder to determine if system should be in safe hold mode.
    for (auto &device : State::Hardware::devices) {
        Devices::Device &dptr = device.second;
        State::Hardware::DeviceState dstate = State::read(State::Hardware::hat.at(device.first), State::Hardware::hardware_state_lock);
        if (!dstate.is_functional && !dstate.error_ignored) {
            debug_printf("Detected SAFE HOLD condition due to failure of device: %s\n", dptr.name().c_str());
            reason = 1;
        }
    }

    // TODO add more software checks

    return reason;
}