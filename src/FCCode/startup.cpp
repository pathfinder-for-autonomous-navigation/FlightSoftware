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
#include "controllers/constants.hpp"
#include "state/EEPROMAddresses.hpp"
#include "state/state_holder.hpp"
#include "state/fault_state_holder.hpp"
#include <rwmutex.hpp>
#include "startup.hpp"
#include "debug.hpp"
#include "deployment_timer.hpp"
//#include "data_collection/adcs_threads.h"

// thread_t* deployment_timer_thread;
namespace RTOSTasks {
    // thread_t* master_thread;
    // thread_t* gomspace_thread;
    thread_t* piksi_thread;
    thread_t* system_output_thread;
    // thread_t* quake_thread;
    // thread_t* adcs_thread;
    thread_t* gnc_thread;
    // thread_t* propulsion_thread;
}
using namespace RTOSTasks;

static void start_satellite_processes() {
    // Start up satellite processes
    // dbg.println(debug_console::severity::INFO, "Starting ADCS controller process.");
    // adcs_thread = chThdCreateStatic(adcs_controller_workingArea, sizeof(adcs_controller_workingArea), 
    //     adcs_thread_priority, adcs_controller, NULL);
    
    // dbg.println(debug_console::severity::INFO, "Starting Gomspace controller process.");
    // gomspace_thread = chThdCreateStatic(gomspace_controller_workingArea, sizeof(gomspace_controller_workingArea), 
    //     gomspace_thread_priority, gomspace_controller, NULL);
    
    dbg.println(debug_console::severity::INFO, "Starting Piksi controller process.");
    piksi_thread = chThdCreateStatic(piksi_controller_workingArea, sizeof(piksi_controller_workingArea),
                                     piksi_thread_priority, piksi_controller, NULL);

    dbg.println(debug_console::severity::INFO, "Starting GNC calculation controller process.");
    gnc_thread = chThdCreateStatic(gnc_controller_workingArea, sizeof(gnc_controller_workingArea),
        gnc_thread_priority, gnc_controller, NULL);

    dbg.println(debug_console::severity::INFO, "Starting system output controller process.");
    system_output_thread = chThdCreateStatic(system_output_controller_workingArea, sizeof(system_output_controller_workingArea),
                                     system_output_thread_priority, system_output_controller, NULL);
    
    // dbg.println(debug_console::severity::INFO, "Starting propulsion controller process.");
    // propulsion_thread = chThdCreateStatic(propulsion_controller_workingArea, sizeof(propulsion_controller_workingArea), 
    //     propulsion_thread_priority, propulsion_controller, NULL);

    // dbg.println(debug_console::severity::INFO, "Starting Quake radio controller process.");
    // quake_thread = chThdCreateStatic(quake_controller_workingArea, sizeof(quake_controller_workingArea), 
    //     quake_thread_priority, quake_controller, NULL);
    
    // dbg.println(debug_console::severity::INFO, "Starting master controller process.");
    // master_thread = chThdCreateStatic(master_controller_workingArea, sizeof(master_controller_workingArea),
    //     master_thread_priority, master_controller, NULL);

#ifdef DEBUG_ENABLED
    (void)chThdCreateStatic(debug_workingArea, sizeof(debug_workingArea), NORMALPRIO, debug_function, NULL);
#endif
}

void pan_system_setup()
{
    chRegSetThreadName("startup");

#ifdef DEBUG_ENABLED
    dbg.begin();
    debug_eeprom_initialization();
#endif

    dbg.println(debug_console::severity::INFO, "Startup process has begun.");
    initialize_rtos_objects();

    // Determining boot count
    // chMtxLock(&eeprom_lock);
    // unsigned int boot_number = State::read(State::Master::boot_number, State::Master::master_state_lock);
    // EEPROM.get(EEPROM_ADDRESSES::NUM_REBOOTS_H, boot_number);
    // State::write(State::Master::boot_number, boot_number++, State::Master::master_state_lock);
    // EEPROM.put(EEPROM_ADDRESSES::NUM_REBOOTS_H, boot_number);
    // chMtxUnlock(&eeprom_lock);
    // dbg.printf(debug_console::severity::INFO, "This is boot #%d since the satellite left the deployer.", State::Master::boot_number);

    dbg.println(debug_console::severity::INFO, "Initializing hardware setup.");
    hardware_setup();

    dbg.println(debug_console::severity::INFO, "Starting satellite processes.");

    // Start deployment thread
    // deployment_timer_thread = chThdCreateStatic(deployment_timer_workingArea, 
    //         sizeof(deployment_timer_workingArea), RTOSTasks::MAX_THREAD_PRIORITY + 1,
    //         deployment_timer_function, NULL);

    start_satellite_processes();
    //dbg.println(debug_console::severity::INFO, "Starting adcs data collection.");
    //adcs_threads::init();

    chThdSleepSeconds(2);
    /** Silence unwanted threads **/
    dbg.silence_thread(chRegFindThreadByName("adcs"));
    //dbg.silence_thread(chRegFindThreadByName("gnc"));
    dbg.silence_thread(chRegFindThreadByName("gomspace"));
    dbg.silence_thread(chRegFindThreadByName("master"));
    dbg.silence_thread(chRegFindThreadByName("master.safehold_timer"));
    //dbg.silence_thread(chRegFindThreadByName("piksi"));
    dbg.silence_thread(chRegFindThreadByName("propulsion"));
    dbg.silence_thread(chRegFindThreadByName("propulsion.firing"));
    dbg.silence_thread(chRegFindThreadByName("propulsion.venting"));
    dbg.silence_thread(chRegFindThreadByName("propulsion.pressurizing"));
    dbg.silence_thread(chRegFindThreadByName("quake"));
    dbg.silence_thread(chRegFindThreadByName("quake.transceiving"));
    dbg.silence_thread(chRegFindThreadByName("debug"));
    dbg.silence_thread(chRegFindThreadByName("startup.deployment_timer"));
    //dbg.silence_thread(chRegFindThreadByName("startup"));
    //dbg.silence_thread(chRegFindThreadByName("gomspace.power_cycler.adcs"));
    //dbg.silence_thread(chRegFindThreadByName("gomspace.power_cycler.quake"));
    //dbg.silence_thread(chRegFindThreadByName("gomspace.power_cycler.spike_and_hold"));
    dbg.silence_thread(chRegFindThreadByName("gomspace.power_cycler.piksi"));

    dbg.println(debug_console::severity::INFO, "System setup is complete.");
    dbg.println(debug_console::severity::INFO, "Process terminating.");
    chThdExit((msg_t)0);
}

// "UNIT_TEST" used to stop "multiple definition" linker errors when running tests
#ifndef UNIT_TEST
void setup()
{
    chBegin(pan_system_setup);
    while (true);
}

void loop() {}
#endif