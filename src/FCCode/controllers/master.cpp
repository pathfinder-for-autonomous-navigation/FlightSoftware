/** @file master.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains implementation for the master state controller.
 */

#include <utility>
#include <EEPROM.h>
#include <Piksi/GPSTime.hpp>
#include <tensor.hpp>
#include <AttitudeEstimator.hpp>
#include "controllers.hpp"
#include <Device.hpp>
#include "constants.hpp"
#include "../deployment_timer.hpp"
#include "../state/EEPROMAddresses.hpp"
#include "../state/state_holder.hpp"
#include "../comms/apply_uplink.hpp"
#include <AttitudeMath.hpp>

using State::Master::MasterState;
using State::Master::master_state;
using State::Master::PANState;
using State::Master::pan_state;
using State::Master::master_state_lock;

namespace RTOSTasks {
    THD_WORKING_AREA(master_controller_workingArea, 2048);
}

static unsigned short int safe_hold_needed() {
    rwMtxRLock(&master_state_lock);
        bool autoexited_safe_hold = State::Master::autoexited_safe_hold;
    rwMtxRUnlock(&master_state_lock);
    if (autoexited_safe_hold) return 0; // Don't set up safehold in this case.

    unsigned short int reason = 0; // No reason

    rwMtxRLock(&State::Hardware::hat_lock);
    // Checks all flags in Master State Holder to determine if system should be in safe hold mode.
    for (auto &device : State::Hardware::devices) {
        Devices::Device &dptr = device.second;
        State::Hardware::DeviceState &dstate = State::Hardware::hat.at(device.first);
        if (!dstate.is_functional && !dstate.error_ignored) {
            debug_printf("Detected SAFE HOLD condition due to failure of device: %s\n", dptr.name().c_str());
            reason = 1;
        }
    }
    rwMtxRUnlock(&State::Hardware::hat_lock);

    // TODO add more software checks
    rwMtxRLock(&State::Quake::uplink_lock);
        gps_time_t most_recent_uplink_time = State::Quake::most_recent_uplink.time_received;
    rwMtxRLock(&State::Quake::uplink_lock);
    if (State::GNC::get_current_time() - most_recent_uplink_time >= Constants::Quake::UPLINK_TIMEOUT) {
        debug_println("Detected SAFE HOLD condition due to no uplink being received in the last 24 hours.");
        reason = 1; // TODO fix
    }

    return reason;
}

void stop_safe_hold() {
    chThdTerminate(RTOSTasks::safe_hold_timer_thread);
    RTOSTasks::stop_safehold();
}

static void safe_hold(unsigned short int reason) {
    // TODO write reason to state

    debug_println("Entering safe hold mode...");
    rwMtxWLock(&State::Master::master_state_lock);
        State::Master::master_state = State::Master::MasterState::SAFE_HOLD;
        State::Master::pan_state = State::Master::PANState::MASTER_SAFEHOLD;
    rwMtxWUnlock(&State::Master::master_state_lock);

    rwMtxWLock(&State::ADCS::adcs_state_lock);
        State::ADCS::adcs_state = State::ADCS::ZERO_TORQUE;
    rwMtxWUnlock(&State::ADCS::adcs_state_lock);

    rwMtxWLock(&State::Propulsion::propulsion_state_lock);
        State::ADCS::adcs_state = State::ADCS::ZERO_TORQUE;
    rwMtxWUnlock(&State::Propulsion::propulsion_state_lock);

    chMtxLock(&eeprom_lock);
        EEPROM.put(EEPROM_ADDRESSES::SAFE_HOLD_FLAG, true);
    chMtxUnlock(&eeprom_lock);

    // Start safe hold timer
    RTOSTasks::safe_hold_timer_thread = chThdCreateStatic(&RTOSTasks::safe_hold_timer_workingArea, 
                                                    sizeof(RTOSTasks::safe_hold_timer_workingArea), 
                                                    RTOSTasks::master_thread_priority, 
                                                    RTOSTasks::safe_hold_timer, NULL);
}

/**
 * @brief Puts the satellite into the initialization hold mode.
 * 
 * @param reason The failure code that's responsible for entering initialization hold mode.
 */
void initialization_hold(unsigned short int reason) {
    debug_println("Entering initialization hold mode...");
    rwMtxWLock(&master_state_lock);
        // The two state declarations below don't do anything; they're just for cosmetics/maintaining invariants
        master_state = MasterState::INITIALIZATION_HOLD;
        pan_state = PANState::MASTER_INITIALIZATIONHOLD;
    rwMtxWUnlock(&master_state_lock);

    chMtxLock(&eeprom_lock);
        EEPROM.put(EEPROM_ADDRESSES::INITIALIZATION_HOLD_FLAG, true);
    chMtxUnlock(&eeprom_lock);

    if (State::ADCS::angular_rate() >= State::ADCS::MAX_SEMISTABLE_ANGULAR_RATE) {
        rwMtxRLock(&State::Hardware::hat_lock);
            bool is_adcs_working = State::Hardware::hat.at("ADCS").is_functional;
        rwMtxRUnlock(&State::Hardware::hat_lock);
        if (is_adcs_working) {
            rwMtxWLock(&State::ADCS::adcs_state_lock);
                State::ADCS::adcs_state = State::ADCS::ADCSState::ADCS_DETUMBLE;
            rwMtxWUnlock(&State::ADCS::adcs_state_lock);
            chThdEnqueueTimeoutS(&RTOSTasks::adcs_detumbled, S2ST(Constants::Master::INITIALIZATION_HOLD_DETUMBLE_WAIT)); // Wait for detumble to finish.
        }
    }
    // Quake controller will send SOS packet. Control is now handed over to
    // master_loop(), which will continuously check Quake uplink for manual control/mode shift commands
}

static void master_loop() {    
    rwMtxRLock(&State::Quake::uplink_lock);
        bool is_uplink_processed = State::Quake::most_recent_uplink.is_uplink_processed;
    rwMtxRUnlock(&State::Quake::uplink_lock);
    if (!is_uplink_processed) {
        Comms::apply_uplink(State::Quake::most_recent_uplink);
    }

    rwMtxRLock(&master_state_lock);
        MasterState master_state_copy = master_state;
        PANState pan_state_copy = pan_state;
        debug_printf("Master State and PAN State: %d %d\n", master_state_copy, pan_state_copy);
    rwMtxRUnlock(&master_state_lock);
    switch(master_state_copy) {
        case MasterState::DETUMBLE: {
            unsigned short int safe_hold_reason = safe_hold_needed();
            if (safe_hold_reason != 0) safe_hold(safe_hold_reason);
            rwMtxRLock(&State::ADCS::adcs_state_lock);
                State::ADCS::ADCSState adcs_state = State::ADCS::adcs_state;
            rwMtxRUnlock(&State::ADCS::adcs_state_lock);
            if (State::Master::is_deployed && adcs_state != State::ADCS::ADCSState::ADCS_DETUMBLE) {
                rwMtxWLock(&State::ADCS::adcs_state_lock);
                    State::ADCS::adcs_state = State::ADCS::ADCSState::ADCS_DETUMBLE;
                rwMtxWUnlock(&State::ADCS::adcs_state_lock);
            }
            rwMtxRLock(&State::ADCS::adcs_state_lock);
            float angular_rate = State::ADCS::angular_rate();
            rwMtxRUnlock(&State::ADCS::adcs_state_lock);
            if (angular_rate < State::ADCS::MAX_STABLE_ANGULAR_RATE) {
                rwMtxWLock(&State::ADCS::adcs_state_lock);
                    State::ADCS::adcs_state = State::ADCS::ADCSState::POINTING;
                    State::ADCS::cmd_attitude = State::ADCS::cur_attitude;
                rwMtxWUnlock(&State::ADCS::adcs_state_lock);
                rwMtxWLock(&master_state_lock);
                    master_state = MasterState::NORMAL;
                    pan_state = PANState::STANDBY;
                rwMtxWUnlock(&master_state_lock);
            }
        }
        break;
        case MasterState::INITIALIZATION_HOLD:
            // The apply_uplink() is continuously checking uplink for manual control commands and mode shift command
        break;
        case MasterState::NORMAL: {
            unsigned short int safe_hold_reason = safe_hold_needed();
            debug_printf("Safe hold reason: %d\n", safe_hold_reason);
            if (safe_hold_reason != 0) safe_hold(safe_hold_reason);
            switch(pan_state_copy) {
                case PANState::FOLLOWER: {
                    RTOSTasks::LoopTimes::GNC = 60000;
                }
                break;
                case PANState::FOLLOWER_CLOSE_APPROACH: {
                     RTOSTasks::LoopTimes::GNC = 10000;
                    ADCSControllers::point_for_close_approach();
                }
                break;
                case PANState::PAIRED: {
                    // TODO Modify ADCS gains
                    rwMtxWLock(&master_state_lock);
                        pan_state = PANState::STANDBY;
                    rwMtxWUnlock(&master_state_lock);
                }
                break;
                case PANState::STANDBY:
                    ADCSControllers::point_for_standby();
                break;
                case PANState::LEADER_CLOSE_APPROACH:
                    ADCSControllers::point_for_close_approach();
                break;
                case PANState::SPACEJUNK:
                    // Do ABSOLUTELY nothing.
                    // TODO set ADCS to passive
                break;
                default:
                    safe_hold(0); // TODO fix
            }
        }
        break;
        case MasterState::SAFE_HOLD:
            // Don't do anything; this is already being handled by the safe hold callback function!
            break;
        default: {
            debug_printf("%d\n", master_state);
            safe_hold(0); // TODO fix
        }
    }
}

/**
 * @brief Initialize master controller process.
 * 
 * Checks for satellite safehold conditions/previous boot conditions and sets the initial state
 * of the master controller appropriately.
 */
void master_init() {
    chRegSetThreadName("MASTER");
    debug_println("Master controller process has started.");

    // Should we be in safe hold?
    chMtxLock(&eeprom_lock);
        bool previous_boot_safehold = false;
        EEPROM.get(EEPROM_ADDRESSES::SAFE_HOLD_FLAG, previous_boot_safehold);
    chMtxUnlock(&eeprom_lock);
    if (previous_boot_safehold) {
        debug_println("Previous boot ended in safehold mode! The system will start in safehold now.");
        safe_hold(0); // TODO fix
        return;
    }

    chMtxLock(&eeprom_lock);
        bool prevboot_initialization_hold = false;
        EEPROM.get(EEPROM_ADDRESSES::INITIALIZATION_HOLD_FLAG, prevboot_initialization_hold);
    chMtxUnlock(&eeprom_lock);
    if (prevboot_initialization_hold) {
        debug_println("Previous boot ended in initialization hold mode! The system will start in initialization hold now.");
        initialization_hold(0); // TODO fix
        return;
    }

    debug_println("Previous boot did not end in initialization or safe hold mode. Checking to see if a hold is necessary...");
    if (safe_hold_needed()) {
        rwMtxRLock(&master_state_lock);
            unsigned int boot_number = State::Master::boot_number;
        rwMtxRUnlock(&master_state_lock);
        if (boot_number == 1)
            initialization_hold(0); // TODO fix
        else
            safe_hold(0); // TODO fix
    }
    else {
        debug_println("Proceeding to normal boot.");
        rwMtxWLock(&State::Master::master_state_lock);
            master_state = MasterState::DETUMBLE;
            pan_state = PANState::MASTER_DETUMBLE;
        rwMtxWUnlock(&State::Master::master_state_lock);
    }
}

void RTOSTasks::master_controller(void *args) {
    master_init();

    systime_t time = chVTGetSystemTimeX(); // T0
    while (true) {
        time += MS2ST(RTOSTasks::LoopTimes::MASTER);
        master_loop();
        chThdSleepUntil(time);
    }
}