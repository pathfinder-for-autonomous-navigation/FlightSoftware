#include "power_cyclers.hpp"
#include <EEPROM.h>
#include "../../state/EEPROMAddresses.hpp"
#include "../../state/device_states.hpp"
#include "../../state/state_holder.hpp"

namespace Gomspace {
    thread_t* adcs_system_thread;
    thread_t* spike_and_hold_thread;
    thread_t* piksi_thread;
    thread_t* quake_thread;
}
using namespace Devices;

MEMORYPOOL_DECL(Gomspace::power_cycler_pool, 1024, NULL);

THD_FUNCTION(Gomspace::cycler_fn, args) {
    cycler_arg_t* cycler_args = (cycler_arg_t*) args;

    debug_println("Incrementing cycle count.");
    rwMtxWLock(&State::Hardware::hardware_state_lock);
        State::Hardware::hat.at(cycler_args->device).boot_count = 
            State::Hardware::hat.at(cycler_args->device).boot_count++;
    rwMtxWUnlock(&State::Hardware::hardware_state_lock);
    chMtxLock(&eeprom_lock);
        unsigned int boot_count;
        if (cycler_args->device == piksi) {
            EEPROM.get(EEPROM_ADDRESSES::DEVICE_REBOOTS_PIKSI, boot_count);
            EEPROM.put(EEPROM_ADDRESSES::DEVICE_REBOOTS_PIKSI, ++boot_count);
        }
        else if (cycler_args->device == quake) {
            EEPROM.get(EEPROM_ADDRESSES::DEVICE_REBOOTS_QUAKE, boot_count);
            EEPROM.put(EEPROM_ADDRESSES::DEVICE_REBOOTS_QUAKE, ++boot_count);
        }
        else if (cycler_args->device == adcs_system) {
            EEPROM.get(EEPROM_ADDRESSES::DEVICE_REBOOTS_ADCS, boot_count);
            EEPROM.put(EEPROM_ADDRESSES::DEVICE_REBOOTS_ADCS, ++boot_count);
        }
        else if (cycler_args->device == spike_and_hold) {
            EEPROM.get(EEPROM_ADDRESSES::DEVICE_REBOOTS_SPIKE_AND_HOLD, boot_count);
            EEPROM.put(EEPROM_ADDRESSES::DEVICE_REBOOTS_SPIKE_AND_HOLD, ++boot_count);
        }
    chMtxUnlock(&eeprom_lock);
    
    debug_println("Checking if Gomspace is functional.");
    chMtxLock(cycler_args->device_lock);
        if (State::Hardware::check_is_functional(gomspace)) {
            debug_println("Gomspace is functional. Turning off device.");
            gomspace->set_single_output(cycler_args->pin, 0);
            State::write(State::Hardware::hat.at(cycler_args->device).powered_on, 
                false, State::Hardware::hardware_state_lock);
            State::write(State::Hardware::hat.at(cycler_args->device).is_functional, 
                false, State::Hardware::hardware_state_lock);

            // Wait for output capacitor voltage to go all the way down to zero.
            chThdSleepSeconds(30);

            debug_println("Turning on device.");
            gomspace->set_single_output(cycler_args->pin, 1);
            chThdSleepMilliseconds(10);

            debug_println("Writing device status to state.");
            State::write(State::Hardware::hat.at(cycler_args->device).powered_on, 
                true, State::Hardware::hardware_state_lock);
            State::write(State::Hardware::hat.at(cycler_args->device).is_functional, 
                (cycler_args->device)->is_functional(), State::Hardware::hardware_state_lock);

            debug_println("Completed power cycle.");
        }
        else debug_println("Gomspace is nonfunctional.");
    chMtxUnlock(cycler_args->device_lock);

    chThdExit((msg_t) 0);
}