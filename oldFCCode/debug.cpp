/** @file debug.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains definitions of functions that facilitate printing for
 * debugging.
 */

#include "debug.hpp"
#include <Arduino.h>
#include <EEPROM.h>
#include <stdarg.h>
#include <unordered_set>
#include "controllers/controllers.hpp"
#include "state/EEPROMAddresses.hpp"

void debug_eeprom_initialization() {
    chSysLock();
    EEPROM.put(EEPROM_ADDRESSES::NUM_REBOOTS_H, (unsigned int)1);
    EEPROM.put(EEPROM_ADDRESSES::DEPLOYMENT, false);
    EEPROM.put(EEPROM_ADDRESSES::DEPLOYMENT_TIMER, (unsigned int)0);
    EEPROM.put(EEPROM_ADDRESSES::INITIALIZATION_HOLD_FLAG, false);
    EEPROM.put(EEPROM_ADDRESSES::SAFE_HOLD_FLAG, false);
    chSysUnlock();
}

/** @brief Working area for main debugger thread process. */
THD_WORKING_AREA(debug_workingArea, 4096);

/** @brief Main debugger thread process.
 * This process can be used to produce arbitrary debugging data. For example, we
 * can print stack sizes in this process--it wouldn't make sense to try to fit
 * that kind of data in any other process, so we put it in this process instead.
 * */
void debug_function(void *arg) {
    chRegSetThreadName("debug");

    dbg.println(debug_severity::INFO, "Debug thread initializing.");
    while (true) {
        // Use this to report stack space and stuff.
        dbg.println(debug_severity::DEBUG, "Stack space remaining:");
        dbg.printf(debug_severity::DEBUG, "ADCS: %d",
                   chUnusedThreadStack(RTOSTasks::adcs_controller_workingArea,
                                       sizeof(RTOSTasks::adcs_controller_workingArea)));
        dbg.printf(debug_severity::DEBUG, "Gomspace: %d",
                   chUnusedThreadStack(RTOSTasks::gomspace_controller_workingArea,
                                       sizeof(RTOSTasks::gomspace_controller_workingArea)));
        dbg.printf(debug_severity::DEBUG, "Master: %d",
                   chUnusedThreadStack(RTOSTasks::master_controller_workingArea,
                                       sizeof(RTOSTasks::master_controller_workingArea)));
        dbg.printf(debug_severity::DEBUG, "Piksi: %d",
                   chUnusedThreadStack(RTOSTasks::piksi_controller_workingArea,
                                       sizeof(RTOSTasks::piksi_controller_workingArea)));
        dbg.printf(debug_severity::DEBUG, "Propulsion: %d",
                   chUnusedThreadStack(RTOSTasks::propulsion_controller_workingArea,
                                       sizeof(RTOSTasks::propulsion_controller_workingArea)));
        dbg.printf(debug_severity::DEBUG, "Quake: %d",
                   chUnusedThreadStack(RTOSTasks::quake_controller_workingArea,
                                       sizeof(RTOSTasks::quake_controller_workingArea)));
        dbg.blink_led();
        chThdSleepSeconds(5);
    }
}