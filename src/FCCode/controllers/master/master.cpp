/** @file master.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains implementation for the master state controller.
 */

#include <EEPROM.h>
#include "master_helpers.hpp"
#include "../constants.hpp"
#include "../adcs/adcs_helpers.hpp"
#include "../../state/EEPROMAddresses.hpp"
#include "../../state/state_holder.hpp"
#include "../../comms/apply_uplink.hpp"

using State::Master::MasterState;
using State::Master::master_state;
using State::Master::PANState;
using State::Master::pan_state;
using State::Master::master_state_lock;

namespace RTOSTasks {
    THD_WORKING_AREA(master_controller_workingArea, 2048);
}

static void master_loop() {    
    Comms::apply_uplink_data(State::Quake::most_recent_uplink);

    MasterState master_state_copy = State::read(master_state, master_state_lock);
    PANState pan_state_copy = State::read(pan_state, master_state_lock);
    debug_printf("Master State and PAN State: %d %d\n", master_state_copy, pan_state_copy);
    switch(master_state_copy) {
        case MasterState::DETUMBLE: {
            chMtxLock(&eeprom_lock);
                EEPROM.put(EEPROM_ADDRESSES::FINAL_STATE_FLAG, (unsigned char) 0);
            chMtxUnlock(&eeprom_lock);
            unsigned short int safe_hold_reason = Master::safe_hold_needed();
            if (safe_hold_reason != 0) Master::safe_hold(safe_hold_reason);
            
            State::ADCS::ADCSState adcs_state = State::read(State::ADCS::adcs_state, State::ADCS::adcs_state_lock);
            
            if (State::Master::is_deployed && adcs_state != State::ADCS::ADCSState::ADCS_DETUMBLE) {
                State::write(State::ADCS::adcs_state, 
                    State::ADCS::ADCSState::ADCS_DETUMBLE, State::ADCS::adcs_state_lock);
            }

            if (State::ADCS::angular_rate() < Constants::ADCS::MAX_STABLE_ANGULAR_RATE) {
                State::write(State::ADCS::adcs_state, 
                    State::ADCS::ADCSState::POINTING, State::ADCS::adcs_state_lock);
                State::write(State::ADCS::cmd_attitude, 
                    State::ADCS::cur_attitude, State::ADCS::adcs_state_lock);
                State::write(master_state, MasterState::NORMAL, master_state_lock);
                State::write(pan_state, PANState::STANDBY, master_state_lock);
            }
        }
        break;
        case MasterState::INITIALIZATION_HOLD:
            // The apply_uplink() is continuously checking uplink for manual control commands and mode shift command
        break;
        case MasterState::NORMAL: {
            unsigned short int safe_hold_reason = Master::safe_hold_needed();
            debug_printf("Safe hold reason: %d\n", safe_hold_reason);
            if (safe_hold_reason != 0) Master::safe_hold(safe_hold_reason);
            switch(pan_state_copy) {
                case PANState::FOLLOWER: {
                    chMtxLock(&eeprom_lock);
                        EEPROM.put(EEPROM_ADDRESSES::IS_FOLLOWER, true);
                    chMtxUnlock(&eeprom_lock);
                    State::write(State::Master::is_follower, true, State::Master::master_state_lock);
                    RTOSTasks::LoopTimes::GNC = 60000;
                }
                break;
                case PANState::FOLLOWER_CLOSE_APPROACH: {
                    RTOSTasks::LoopTimes::GNC = 10000;
                    ADCSControllers::point_for_close_approach();
                }
                break;
                case PANState::STANDBY:
                    Comms::apply_uplink_commands(State::Quake::most_recent_uplink);
                    chMtxLock(&eeprom_lock);
                        EEPROM.put(EEPROM_ADDRESSES::FINAL_STATE_FLAG, (unsigned char) 0);
                    chMtxUnlock(&eeprom_lock);
                    State::write(State::ADCS::adcs_state, 
                        State::ADCS::ADCSState::POINTING, State::ADCS::adcs_state_lock);
                    State::write(State::Propulsion::propulsion_state, 
                        State::Propulsion::PropulsionState::IDLE, State::Propulsion::propulsion_state_lock);
                    ADCSControllers::point_for_standby();
                break;
                case PANState::LEADER_CLOSE_APPROACH:
                    ADCSControllers::point_for_close_approach();
                break;
                case PANState::DOCKING: {
                    chMtxLock(&eeprom_lock);
                        EEPROM.put(EEPROM_ADDRESSES::FINAL_STATE_FLAG, (unsigned char) 1);
                    chMtxUnlock(&eeprom_lock);
                    State::write(State::ADCS::adcs_state, 
                        State::ADCS::ADCSState::ZERO_TORQUE, State::ADCS::adcs_state_lock);
                    State::write(State::Propulsion::propulsion_state, 
                        State::Propulsion::PropulsionState::DISABLED, State::Propulsion::propulsion_state_lock);

                    chVTDoSetI(&Master::docking_timer, S2ST(Constants::Master::DOCKING_TIMEOUT), Master::stop_docking_mode, NULL);
                    if (Devices::docking_switch.pressed() && State::Hardware::can_get_data(Devices::docking_switch)) {
                        State::write(State::Master::pan_state, PANState::DOCKED, master_state_lock);
                    }
                }
                break;
                case PANState::DOCKED: {
                    chMtxLock(&eeprom_lock);
                        EEPROM.put(EEPROM_ADDRESSES::FINAL_STATE_FLAG, (unsigned char) 2);
                    chMtxUnlock(&eeprom_lock);
                    // Do nothing, just wait for ground command
                }
                break;
                case PANState::PAIRED: {
                    // TODO Modify ADCS gains
                    State::write(pan_state, PANState::STANDBY, master_state_lock);
                }
                break;
                case PANState::SPACEJUNK: {
                    chMtxLock(&eeprom_lock);
                        EEPROM.put(EEPROM_ADDRESSES::FINAL_STATE_FLAG, (unsigned char) 4);
                    chMtxUnlock(&eeprom_lock);
                    // Point in the same direction in ECI. Momentum is slowly dumped by the
                    // pointing function.
                    State::write(State::ADCS::cmd_attitude, State::ADCS::cur_attitude, State::ADCS::adcs_state_lock);
                    State::write(State::ADCS::adcs_state, State::ADCS::ADCSState::POINTING, State::ADCS::adcs_state_lock);
                }
                break;
                default:
                    Master::safe_hold(0); // TODO fix
            }
        }
        break;
        case MasterState::SAFE_HOLD:
            // Don't do anything; this is already being handled by the safe hold callback function!
            break;
        default: {
            debug_printf("%d\n", master_state);
            Master::safe_hold(0); // TODO fix
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

    chVTObjectInit(&Master::docking_timer);

    // Should we be in safe hold?
    chMtxLock(&eeprom_lock);
        bool previous_boot_safehold = false;
        EEPROM.get(EEPROM_ADDRESSES::SAFE_HOLD_FLAG, previous_boot_safehold);
    chMtxUnlock(&eeprom_lock);
    if (previous_boot_safehold) {
        debug_println("Previous boot ended in safehold mode! The system will start in safehold now.");
        Master::safe_hold(0); // TODO fix
        return;
    }

    chMtxLock(&eeprom_lock);
        bool prevboot_initialization_hold = false;
        EEPROM.get(EEPROM_ADDRESSES::INITIALIZATION_HOLD_FLAG, prevboot_initialization_hold);
    chMtxUnlock(&eeprom_lock);
    if (prevboot_initialization_hold) {
        debug_println("Previous boot ended in initialization hold mode! The system will start in initialization hold now.");
        Master::initialization_hold(0); // TODO fix
        return;
    }

    debug_println("Previous boot did not end in initialization or safe hold mode. Checking to see if a hold is necessary...");
    if (Master::safe_hold_needed()) {
        unsigned int boot_number = State::read(State::Master::boot_number, master_state_lock);
        if (boot_number == 1)
            Master::initialization_hold(0); // TODO fix
        else
            Master::safe_hold(0); // TODO fix
    }
    else {
        debug_println("Proceeding to normal boot.");
        chMtxLock(&eeprom_lock);
            bool is_follower = EEPROM.read(EEPROM_ADDRESSES::IS_FOLLOWER);
            unsigned char final_state = EEPROM.read(EEPROM_ADDRESSES::FINAL_STATE_FLAG);
        chMtxUnlock(&eeprom_lock);
        rwMtxWLock(&State::Master::master_state_lock);
            if (final_state != 0) {
                State::Master::is_follower = is_follower;
            }
            if (final_state == 1) {
                master_state = MasterState::NORMAL;
                pan_state = PANState::DOCKING;
            }
            else if (final_state == 2) {
                master_state = MasterState::NORMAL;
                pan_state = PANState::DOCKED;
            }
            else if(final_state == 3) {
                master_state = MasterState::NORMAL;
                pan_state = PANState::PAIRED;
            }
            else if (final_state == 4) {
                master_state = MasterState::NORMAL;
                pan_state = PANState::SPACEJUNK;
            }
            else {
                master_state = MasterState::DETUMBLE;
                pan_state = PANState::MASTER_DETUMBLE;
            }
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