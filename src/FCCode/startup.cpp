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

// thread_t* deployment_timer_thread;
namespace RTOSTasks {
    // thread_t* master_thread;
    // thread_t* gomspace_thread;
    thread_t* piksi_thread;
    // thread_t* quake_thread;
    // thread_t* adcs_thread;
    thread_t* gnc_thread;
    // thread_t* propulsion_thread;
}
using namespace RTOSTasks;

static void hardware_setup() {
    rwMtxObjectInit(&State::Hardware::hardware_state_lock);

    debug_println("Initializing hardware buses.");
    // Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000, I2C_OP_MODE_IMM); // Gomspace

    debug_println("Initializing hardware peripherals.");
    for (auto device : State::Hardware::hat) {
        Devices::Device* dptr = device.first;
        debug_printf("Setting up device: %s...", dptr->name().c_str());
        dptr->setup();
        if (dptr->is_functional()) {
            debug_printf_headless("setup was successful!\n");
            State::write((State::Hardware::hat).at(device.first).powered_on, true, State::Hardware::hardware_state_lock);
            State::write((State::Hardware::hat).at(device.first).is_functional, true, State::Hardware::hardware_state_lock);
        }
        else debug_printf_headless("setup was unsuccessful.\n");
    }
}

static void start_satellite_processes() {
    // Start up satellite processes
    // debug_println("Starting ADCS controller process.");
    // adcs_thread = chThdCreateStatic(adcs_controller_workingArea, sizeof(adcs_controller_workingArea), 
    //     adcs_thread_priority, adcs_controller, NULL);
    
    // debug_println("Starting Gomspace controller process.");
    // gomspace_thread = chThdCreateStatic(gomspace_controller_workingArea, sizeof(gomspace_controller_workingArea), 
    //     gomspace_thread_priority, gomspace_controller, NULL);
    
    debug_println("Starting Piksi controller process.");
    piksi_thread = chThdCreateStatic(piksi_controller_workingArea, sizeof(piksi_controller_workingArea), 
        piksi_thread_priority, piksi_controller, NULL);

    debug_println("Starting GNC calculation controller process.");
    gnc_thread = chThdCreateStatic(gnc_controller_workingArea, sizeof(gnc_controller_workingArea),
        gnc_thread_priority, gnc_controller, NULL);
    
    // debug_println("Starting propulsion controller process.");
    // propulsion_thread = chThdCreateStatic(propulsion_controller_workingArea, sizeof(propulsion_controller_workingArea), 
    //     propulsion_thread_priority, propulsion_controller, NULL);

    // debug_println("Starting Quake radio controller process.");
    // quake_thread = chThdCreateStatic(quake_controller_workingArea, sizeof(quake_controller_workingArea), 
    //     quake_thread_priority, quake_controller, NULL);
    
    // debug_println("Starting master controller process.");
    // master_thread = chThdCreateStatic(master_controller_workingArea, sizeof(master_controller_workingArea),
    //     master_thread_priority, master_controller, NULL);

    #ifdef DEBUG
    (void)chThdCreateStatic(debug_workingArea, sizeof(debug_workingArea), NORMALPRIO, debug_function, NULL);
    #endif
}

void pan_system_setup() {
    chRegSetThreadName("STARTUP");

    #ifdef DEBUG
        debug_begin();
        print_pan_logo();
        debug_println_headless("");
        debug_println_headless("Satellite is booting up...");
        debug_println_headless("");
        debug_eeprom_initialization();
    #endif

    debug_println("Startup process has begun.");
    initialize_rtos_objects();

    // Determining boot count
    // chMtxLock(&eeprom_lock);
    // unsigned int boot_number = State::read(State::Master::boot_number, State::Master::master_state_lock);
    // EEPROM.get(EEPROM_ADDRESSES::NUM_REBOOTS_H, boot_number);
    // State::write(State::Master::boot_number, boot_number++, State::Master::master_state_lock);
    // EEPROM.put(EEPROM_ADDRESSES::NUM_REBOOTS_H, boot_number);
    // chMtxUnlock(&eeprom_lock);
    // debug_printf("This is boot #%d since the satellite left the deployer. \n", State::Master::boot_number);

    debug_println("Initializing hardware setup.");
    hardware_setup();

    debug_println("Starting satellite processes.");

    // Start deployment thread
    // deployment_timer_thread = chThdCreateStatic(deployment_timer_workingArea, 
    //         sizeof(deployment_timer_workingArea), RTOSTasks::MAX_THREAD_PRIORITY + 1,
    //         deployment_timer_function, NULL);

    start_satellite_processes();

    debug_println("System setup is complete.");
    debug_println("Process terminating.");
    chThdExit((msg_t)0);
}

// "UNIT_TEST" used to stop "multiple definition" linker errors when running tests
#ifndef UNIT_TEST
void setup() {
    chBegin(pan_system_setup);
    while(true);
}

void loop() {}
#endif