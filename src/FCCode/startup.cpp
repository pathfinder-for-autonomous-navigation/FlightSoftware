/** @file startup.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains the code that is the first to be run when starting up the satellite.
 * This code initializes all of the daughter processes that become responsible for the handling
 * of the satellite during its lifetime.
 */

#include <ChRt.h>
#include <Arduino.h>
#include <rt/chvt.h>
#include <EEPROM.h>
#include "controllers/controllers.hpp"
#include "state/EEPROMAddresses.hpp"
#include "state/state_holder.hpp"
#include <rwmutex.hpp>
#include "debug.hpp"
#include "deployment_timer.hpp"
#include "startup.hpp"

thread_t* deployment_timer_thread;
namespace RTOSTasks {
    thread_t* master_thread;
    thread_t* gomspace_thread;
    thread_t* piksi_thread;
    thread_t* quake_thread;
    thread_t* adcs_thread;
    thread_t* propulsion_thread;
}
using namespace RTOSTasks;

// TODO ensure that RADIOS are NOT turned on during hardware setup.
void hardware_setup() {
    rwMtxObjectInit(&State::Hardware::hat_lock);

    debug_println("Initializing hardware buses.");
    Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000, I2C_OP_MODE_IMM); // Gomspace

    debug_println("Initializing hardware peripherals.");
    for (auto device : State::Hardware::devices) {
        Devices::Device& dptr = device.second;
        debug_printf("Setting up device: %s...", device.first.c_str());
        dptr.setup();
        if (dptr.is_functional()) {
            debug_printf_headless("setup was successful!\n");
            rwMtxWLock(&State::Hardware::hat_lock);
                (State::Hardware::hat).at(device.first).powered_on = true;
                (State::Hardware::hat).at(device.first).is_functional = true;
            rwMtxWUnlock(&State::Hardware::hat_lock);
        }
        else debug_printf_headless("setup was unsuccessful.\n");
    }
}

void set_power_outputs() {
    rwMtxRLock(&State::Master::master_state_lock);
        unsigned int boot_number = State::Master::boot_number;
    rwMtxRUnlock(&State::Master::master_state_lock);

    bool low_power_state = false; // TODO get from Gomspace. If unavailable, _assume_ a low-power state
    if (boot_number == 1 || low_power_state) {
        rwMtxWLock(&State::Hardware::hat_lock);
        if (!low_power_state) {
            // We want to keep the radio on at all costs, so don't turn it off
            // even in a low-power state!
            // TODO turn off Quake from Gomspace
            (State::Hardware::hat).at("Quake").powered_on = true;
            (State::Hardware::hat).at("Quake").error_ignored = false;
        }
            // TODO turn off Piksi from Gomspace
            (State::Hardware::hat).at("Piksi").powered_on = false;
            (State::Hardware::hat).at("Piksi").error_ignored = true;
            // TODO turn off ADCS from Gomspace
            (State::Hardware::hat).at("ADCS").powered_on = false;
            (State::Hardware::hat).at("ADCS").error_ignored = true;
        rwMtxWUnlock(&State::Hardware::hat_lock);
    }
    else {
        // Turn everything on
    }
}

static void start_satellite_processes() {
    // Start up satellite processes
    debug_println("Starting ADCS controller process.");
    adcs_thread = chThdCreateStatic(adcs_controller_workingArea, sizeof(adcs_controller_workingArea), 
        adcs_thread_priority, adcs_controller, NULL);
    
    debug_println("Starting Gomspace controller process.");
    gomspace_thread = chThdCreateStatic(gomspace_controller_workingArea, sizeof(gomspace_controller_workingArea), 
        gomspace_thread_priority, gomspace_controller, NULL);

    debug_println("Starting master controller process.");
    master_thread = chThdCreateStatic(master_controller_workingArea, sizeof(master_controller_workingArea),
        master_thread_priority, master_controller, NULL);
    
    debug_println("Starting Piksi controller process.");
    piksi_thread = chThdCreateStatic(piksi_controller_workingArea, sizeof(piksi_controller_workingArea), 
        piksi_thread_priority, piksi_controller, NULL);
    
    debug_println("Starting propulsion controller process.");
    propulsion_thread = chThdCreateStatic(propulsion_controller_workingArea, sizeof(propulsion_controller_workingArea), 
        propulsion_thread_priority, propulsion_controller, NULL);

    debug_println("Starting Quake radio controller process.");
    quake_thread = chThdCreateStatic(quake_controller_workingArea, sizeof(quake_controller_workingArea), 
        quake_thread_priority, quake_controller, NULL);

    #ifdef DEBUG
    (void)chThdCreateStatic(debug_workingArea, sizeof(debug_workingArea), NORMALPRIO, debug_function, NULL);
    #endif
}

void pan_system_setup() {
    chRegSetThreadName("STARTUP");

    #ifdef DEBUG
        debug_begin();
        print_pan_logo();
        debug_println_headless(""); debug_println_headless("");
        debug_println_headless("Satellite is booting up...");
        debug_println_headless("");
        debug_eeprom_initialization();
    #endif

    debug_println("Startup process has begun.");
    // Initialize all state locks
    chMtxObjectInit(&eeprom_lock);
    rwMtxObjectInit(&State::Hardware::hat_lock);
    rwMtxObjectInit(&State::Master::master_state_lock);
    rwMtxObjectInit(&State::ADCS::adcs_state_lock);
    rwMtxObjectInit(&State::Gomspace::gomspace_state_lock);
    rwMtxObjectInit(&State::Propulsion::propulsion_state_lock);
    rwMtxObjectInit(&State::Piksi::piksi_state_lock);
    rwMtxObjectInit(&State::Quake::quake_state_lock);
    rwMtxObjectInit(&State::Quake::uplink_lock);
    // Initialize all device locks
    chMtxObjectInit(&State::Hardware::adcs_device_lock);
    chMtxObjectInit(&State::Hardware::dcdc_lock);
    chMtxObjectInit(&State::Hardware::spike_and_hold_lock);
    chMtxObjectInit(&State::Hardware::piksi_device_lock);
    chMtxObjectInit(&State::Hardware::gomspace_device_lock);
    chMtxObjectInit(&State::Hardware::quake_device_lock);

    // Determining boot count
    chMtxLock(&eeprom_lock);
    rwMtxWLock(&State::Master::master_state_lock);
        unsigned int boot_number = State::Master::boot_number;
        EEPROM.get(EEPROM_ADDRESSES::NUM_REBOOTS_H, boot_number);
        State::Master::boot_number = boot_number++;
        EEPROM.put(EEPROM_ADDRESSES::NUM_REBOOTS_H, boot_number);
    rwMtxWUnlock(&State::Master::master_state_lock);
    chMtxUnlock(&eeprom_lock);
    debug_printf("This is boot #%d since the satellite left the deployer. \n", State::Master::boot_number);

    debug_println("Initializing hardware setup.");
    set_power_outputs();
    hardware_setup();

    debug_println("Starting satellite processes.");

    // Start deployment thread
    deployment_timer_thread = chThdCreateStatic(deployment_timer_workingArea, 
            sizeof(deployment_timer_workingArea), RTOSTasks::MAX_THREAD_PRIORITY + 1,
            deployment_timer_function, NULL);

    start_satellite_processes();

    debug_println("System setup is complete.");
    debug_println("Process terminating.");
    chThdExit((msg_t)0);
}

// When running tests, we n eed to get rid of setup() and loop(), or C++'s linker complains about multiple definitions!
#ifndef UNIT_TEST
void setup() {
    chBegin(pan_system_setup);
    while(true);
}

void loop() {}
#endif