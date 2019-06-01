/** @file debug.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains definitions of functions that facilitate printing for
 * debugging.
 */

#include <Arduino.h>
#include <stdarg.h>
#include <unordered_set>
#include <EEPROM.h>
#include "state/EEPROMAddresses.hpp"
#include "debug.hpp"
#include "controllers/controllers.hpp"

#ifdef DEBUG

 /** @brief Threads whose output we don't care about, and are therefore silenced.
  * If a thread is commented, that means it is NOT silent.
  */
std::unordered_set<const char *> silenced_threads = {
    "ADCS",
    "ADCS LOOP",
    "ADCS HAT UPDATER",
    // "GNC",
    "GS READ",
    "GOMSPACE",
    "MASTER",
    "SAFEHOLD TIMER",
    // "PIKSI",
    "PROP",
    "QUAKE",
    "DEBUG",
    "DEPLOYMENT",
    // "STARTUP",
    // "PCYCLER:ADCS",
    // "PCYCLER:QUAKE",
    // "PCYCLER:SPH",
    "PCYCLER:PIKSI",
};

void debug_begin() {
    Serial.begin(9600);
    pinMode(13, OUTPUT);
    while (!Serial) {}
}

void debug_eeprom_initialization() {
    chSysLock();
        EEPROM.put(EEPROM_ADDRESSES::NUM_REBOOTS_H, (unsigned int) 1);
        EEPROM.put(EEPROM_ADDRESSES::DEPLOYMENT, false);
        EEPROM.put(EEPROM_ADDRESSES::DEPLOYMENT_TIMER, (unsigned int) 0);
        EEPROM.put(EEPROM_ADDRESSES::INITIALIZATION_HOLD_FLAG, false);
        EEPROM.put(EEPROM_ADDRESSES::SAFE_HOLD_FLAG, false);
    chSysUnlock();
} 

void debug_printf_headless(const char* format, ...) {
    char buf[1024];
    va_list args;
    va_start( args, format );
    chSysLock();
        vsnprintf(buf, sizeof(buf), format, args);
        Serial.print(buf);
        Serial.flush();
    chSysUnlock();
    va_end( args );
}

void debug_printf(const char* format, ...) {
    char buf[1024];
    va_list args;
    va_start( args, format );
    vsnprintf(buf, sizeof(buf), format, args);
    const char* name = chThdGetSelfX()->name;
    if (silenced_threads.count(name) == 0) {
        chSysLock();
            Serial.printf("[%s] ", name);
            Serial.print(buf);
            Serial.flush();
        chSysUnlock();
    }
    va_end( args );
}

void debug_println(const char* str) {
    const char* name = chThdGetSelfX()->name;
    if (silenced_threads.count(name) == 0) {
        chSysLock();
            Serial.printf("[%s] ", name);
            debug_println_headless(str);
        chSysUnlock();
    }
}

void debug_println_headless(const char* str) {
    chSysLock();
        Serial.println(str);
        Serial.flush();
    chSysUnlock();
}

void debug_println_headless(void) {
    chSysLock();
        Serial.println();
        Serial.flush();
    chSysUnlock();
}

/** @brief Blink an LED on and then off.
 * Useful for critical situations where a print to the serial console
 * would upset a timing condition.
 * */
void debug_blink_led() {
    digitalWrite(13, HIGH);
    chThdSleepMilliseconds(500);
    digitalWrite(13, LOW);
    chThdSleepMilliseconds(500);
}

void print_pan_logo() {
    Serial.println(" _______     _       ____  _____  ");
    Serial.println("|_   __ \\   / \\     |_   \\|_   _| ");
    Serial.println("  | |__) | / _ \\      |   \\ | |   ");
    Serial.println("  |  ___/ / ___ \\     | |\\ \\| |   ");
    Serial.println(" _| |_  _/ /   \\ \\_  _| |_\\   |_  ");
    Serial.println("|_____||____| |____||_____|\\____| ");
    Serial.println("");
    Serial.println("Pathfinder for Autonomous Navigation");
    Serial.println("Space Systems Design Studio, Cornell Unversity");
    Serial.println("Flight Code | Version 1.0 | Tanishq Aggarwal (ta335@cornell.edu)");
    Serial.println("                            Kyle Krol (kpk36@cornell.edu)");
    Serial.println("                            Nathan Zimmerberg (nhz2@cornell.edu)");
    Serial.println("                            Athena Cheung (afc65@cornell.edu)");
    Serial.println("                            Anusha Choudhury (ac978@cornell.edu)");
    Serial.println();
    Serial.println("Satellite is booting up...");
    Serial.println();
    Serial.flush();
}

/** @brief Working area for main debugger thread process. */
THD_WORKING_AREA(debug_workingArea, 4096);

/** @brief Main debugger thread process.
 * This process can be used to produce arbitrary debugging data. For example, we can print
 * stack sizes in this process--it wouldn't make sense to try to fit that kind of data in
 * any other process, so we put it in this process instead.
 * */
void debug_function(void *arg) {
    chRegSetThreadName("DEBUG");

    debug_println("Debug thread initializing.");
    while(true) {
        // Use this to report stack space and stuff.
        debug_println("Stack space remaining:");
        debug_printf("ADCS: %d\n", chUnusedThreadStack(RTOSTasks::adcs_controller_workingArea, sizeof(RTOSTasks::adcs_controller_workingArea)));
        debug_printf("Gomspace: %d\n", chUnusedThreadStack(RTOSTasks::gomspace_controller_workingArea, sizeof(RTOSTasks::gomspace_controller_workingArea)));
        debug_printf("Master: %d\n", chUnusedThreadStack(RTOSTasks::master_controller_workingArea, sizeof(RTOSTasks::master_controller_workingArea)));
        debug_printf("Piksi: %d\n", chUnusedThreadStack(RTOSTasks::piksi_controller_workingArea, sizeof(RTOSTasks::piksi_controller_workingArea)));
        debug_printf("Propulsion: %d\n", chUnusedThreadStack(RTOSTasks::propulsion_controller_workingArea, sizeof(RTOSTasks::propulsion_controller_workingArea)));
        debug_printf("Quake: %d\n", chUnusedThreadStack(RTOSTasks::quake_controller_workingArea, sizeof(RTOSTasks::quake_controller_workingArea)));
        debug_blink_led();
        chThdSleepSeconds(5);
    }
}

#else
void debug_begin() {}
void debug_eeprom_initialization() {}
void debug_printf_headless(const char* format, ...) {}
void debug_printf(const char* format, ...) {}
void debug_println(const char* str) {}
void debug_println_headless(const char* str) {}
void debug_blink_led() {}
void print_pan_logo() {}
#endif
