/** @file controllers.hpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains definitions for thread-related data for each of the controllers.
 */

#include <ChRt.h>
#include "../debug.hpp"

#ifndef TASKS_HPP_
#define TASKS_HPP_

namespace RTOSTasks {
    //! ADCS state controller process
    extern THD_FUNCTION(adcs_controller, arg);
    //! Gomspace state controller process
    extern THD_FUNCTION(gomspace_controller, arg);
    //! Master state controller process
    extern THD_FUNCTION(master_controller, arg);
    //! Piksi state controller process
    extern THD_FUNCTION(piksi_controller, arg);
    //! Propulsion state controller process
    extern THD_FUNCTION(propulsion_controller, arg);
    //! Quake state controller process
    extern THD_FUNCTION(quake_controller, arg);

    //! Working area for ADCS state controller process
    extern THD_WORKING_AREA(adcs_controller_workingArea, 8192);
    //! Working area for Gomspace state controller process
    extern THD_WORKING_AREA(gomspace_controller_workingArea, 4096);
    //! Working area for master state controller process
    extern THD_WORKING_AREA(master_controller_workingArea, 8192);
    //! Working area for Piksi state controller process
    extern THD_WORKING_AREA(piksi_controller_workingArea, 4096);
    //! Working area for propulsion state controller process
    extern THD_WORKING_AREA(propulsion_controller_workingArea, 4096);
    //! Working area for Quake state controller process
    extern THD_WORKING_AREA(quake_controller_workingArea, 16384);

    //! Pointer to the ADCS controller's thread object
    extern thread_t* adcs_thread;
    //! Pointer to the Gomspace controller's thread object
    extern thread_t* gomspace_thread;
    //! Pointer to the master controller's thread object
    extern thread_t* master_thread;
    //! Pointer to the Piksi controller's thread object
    extern thread_t* piksi_thread;
    //! Pointer to the propulsion controller's thread object
    extern thread_t* propulsion_thread;
    //! Pointer to the Quake controller's thread object
    extern thread_t* quake_thread;
    
    //! Minimum possible priority of any satellite thread
    constexpr tprio_t MIN_THREAD_PRIORITY = NORMALPRIO + 1;
    //! Maximum possible priority of any satellite thread
    constexpr tprio_t MAX_THREAD_PRIORITY = NORMALPRIO + 6;

    //! Priority of ADCS controller's thread
    constexpr tprio_t adcs_thread_priority = MAX_THREAD_PRIORITY;
    //! Priority of Gomspace controller's thread
    constexpr tprio_t gomspace_thread_priority = MAX_THREAD_PRIORITY;
    //! Priority of master controller's thread
    constexpr tprio_t master_thread_priority = MAX_THREAD_PRIORITY;
    //! Priority of Piksi controller's thread
    constexpr tprio_t piksi_thread_priority = MAX_THREAD_PRIORITY;
    //! Priority of propulsion controller's thread
    constexpr tprio_t propulsion_thread_priority = MAX_THREAD_PRIORITY;
    //! Priority of Quake controller's thread
    constexpr tprio_t quake_thread_priority = MAX_THREAD_PRIORITY;

    //! Loop times for each process running within the satellite.
    class LoopTimes {
      public:
        // TODO fix all
        //! Number of milliseconds between ADCS state controller iterations
        static constexpr unsigned int ADCS = 100;
        //! Number of milliseconds between ADCS HAT checking iterations
        static constexpr unsigned int ADCS_HAT_CHECK = 1000;
        //! Number of milliseconds between Gomspace state controller iterations
        static constexpr unsigned int GOMSPACE = 1000;
        //! Number of milliseconds between master state controller iterations
        static constexpr unsigned int MASTER = 1000;
        //! Number of milliseconds between Piksi state controller iterations
        static constexpr unsigned int PIKSI = 1000;
        //! Number of milliseconds between propulsion actuations. This is really high
        // so that the pressure reading has time to stabilize.
        static constexpr unsigned int PROPULSION_ACTUATION_LOOP = 30000;
        //! Number of milliseconds between propulsion repressurization/firing loop iterations
        static constexpr unsigned int PROPULSION_LOOP = 1000;
        #ifdef DEBUG
        //! Number of milliseconds between quake controller iterations
        static constexpr unsigned int QUAKE = 5000;
        #else
        //! Number of milliseconds between quake controller iterations
        static constexpr unsigned int QUAKE = 300000;
        #endif
    };

    // Master-specific
    //! Master safe hold wait process working area
    extern THD_WORKING_AREA(safe_hold_timer_workingArea, 2048);
    //! Master safe hold wait process
    extern THD_FUNCTION(safe_hold_timer, arg);
    //! Master safe hold wait process thread pointer
    extern thread_t* safe_hold_timer_thread;
    //! Stops safehold forcibly
    extern void stop_safehold();

    // ADCS specific
    //! Threads waiting on a finish of ADCS detumbling
    extern threads_queue_t adcs_detumbled;
    //! Threads waiting on a finish of an ADCS pointing instruction
    extern threads_queue_t adcs_pointing_accomplished;

    // Propulsion-specific
    void disable_thruster_firing();

    // Quake specific
    #ifdef DEBUG
    //! Milliseconds before the uplink/downlink deadline that the quake process must finish trying to send a downlink
    static constexpr unsigned int TRY_DOWNLINK_UNTIL = 1000; 
    //! Milliseconds before the uplink/downlink deadline that the quake process must finish trying to read an uplink
    static constexpr unsigned int TRY_UPLINK_UNTIL = 500;
    //! Milliseconds to wait between Quake send/receive retries
    static constexpr unsigned int WAIT_BETWEEN_RETRIES = 50;
    #else
    //! Milliseconds before the uplink/downlink deadline that the quake process must finish trying to send a downlink
    static constexpr unsigned int TRY_DOWNLINK_UNTIL = 30000; 
    //! Milliseconds before the uplink/downlink deadline that the quake process must finish trying to read an uplink
    static constexpr unsigned int TRY_UPLINK_UNTIL = 500;
    //! Milliseconds to wait between Quake send/receive retries
    static constexpr unsigned int WAIT_BETWEEN_RETRIES = 500; 
    #endif
}

#endif