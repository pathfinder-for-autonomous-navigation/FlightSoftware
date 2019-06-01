/** @file controllers.hpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains definitions for thread-related data for each of the controllers.
 */

#include <ChRt.h>
#include <rwmutex.hpp>
#include "../debug.hpp"

#ifndef TASKS_HPP_
#define TASKS_HPP_

namespace RTOSTasks {
    //! Start lock objects used by processes
    void initialize_rtos_objects();

    //! ADCS state controller process
    extern THD_FUNCTION(adcs_controller, arg);
    //! Gomspace state controller process
    extern THD_FUNCTION(gomspace_controller, arg);
    //! Master state controller process
    extern THD_FUNCTION(master_controller, arg);
    //! GNC calculation controller process
    extern THD_FUNCTION(gnc_controller, arg);
    //! Piksi state controller process
    extern THD_FUNCTION(piksi_controller, arg);
    //! System Output controller process
    extern THD_FUNCTION(system_output_controller, arg);
    //! Propulsion state controller process
    extern THD_FUNCTION(propulsion_controller, arg);
    //! Quake state controller process
    extern THD_FUNCTION(quake_controller, arg);

    //! Working area for ADCS state controller process
    extern THD_WORKING_AREA(adcs_controller_workingArea, 2048);
    //! Working area for Gomspace state controller process
    extern THD_WORKING_AREA(gomspace_controller_workingArea, 2048);
    //! Working area for master state controller process
    extern THD_WORKING_AREA(master_controller_workingArea, 2048);
    //! Working area for GNC calculation process
    extern THD_WORKING_AREA(gnc_controller_workingArea, 2048);
    //! Working area for Piksi state controller process
    extern THD_WORKING_AREA(piksi_controller_workingArea, 2048);
    //! Working area for system output state controller process
    extern THD_WORKING_AREA(system_output_controller_workingArea, 2048);
    //! Working area for propulsion state controller process
    extern THD_WORKING_AREA(propulsion_controller_workingArea, 2048);
    //! Working area for Quake state controller process
    extern THD_WORKING_AREA(quake_controller_workingArea, 4096);

    //! Pointer to the ADCS controller's thread object
    extern thread_t* adcs_thread;
    //! Pointer to the Gomspace controller's thread object
    extern thread_t* gomspace_thread;
    //! Pointer to the master controller's thread object
    extern thread_t* master_thread;
    //! Pointer to the GNC calculation controller's thread object
    extern thread_t* gnc_thread;
    //! Pointer to the Piksi controller's thread object
    extern thread_t* piksi_thread;
    //! Pointer to the System Output controller's thread object
    extern thread_t* piksi_thread;
    //! Pointer to the propulsion controller's thread object
    extern thread_t* propulsion_thread;
    //! Pointer to the Quake controller's thread object
    extern thread_t* quake_thread;
    
    //! Minimum possible priority of any satellite thread
    constexpr tprio_t MIN_THREAD_PRIORITY = NORMALPRIO + 1;
    //! Maximum possible priority of any satellite thread
    constexpr tprio_t MAX_THREAD_PRIORITY = NORMALPRIO + 6;

    // TODO fix all
    //! Priority of ADCS controller's thread
    constexpr tprio_t adcs_thread_priority = MAX_THREAD_PRIORITY;
    //! Priority of Gomspace controller's thread
    constexpr tprio_t gomspace_thread_priority = MAX_THREAD_PRIORITY;
    //! Priority of master controller's thread
    constexpr tprio_t master_thread_priority = MAX_THREAD_PRIORITY;
    //! Priority of GNC controller's thread
    constexpr tprio_t gnc_thread_priority = MAX_THREAD_PRIORITY;
    //! Priority of Piksi controller's thread
    constexpr tprio_t piksi_thread_priority = MAX_THREAD_PRIORITY;
    //! Priority of Piksi controller's thread
    constexpr tprio_t system_output_thread_priority = MAX_THREAD_PRIORITY;
    //! Priority of propulsion controller's thread
    constexpr tprio_t propulsion_thread_priority = MAX_THREAD_PRIORITY;
    //! Priority of Quake controller's thread
    constexpr tprio_t quake_thread_priority = MAX_THREAD_PRIORITY;

    //! Loop times for each process running within the satellite.
    class LoopTimes {
      public:
        //! Number of milliseconds between ADCS state controller iterations
        static constexpr unsigned int ADCS = 100;
        //! Number of milliseconds between ADCS HAT checking iterations
        static constexpr unsigned int ADCS_HAT_CHECK = 50;
        //! Number of milliseconds between Gomspace state controller iterations
        static constexpr unsigned int GOMSPACE = 1000;
        //! Number of milliseconds between master state controller iterations
        static constexpr unsigned int MASTER = 1000;
        //! Number of milliseconds between GNC calculation iterations
        static unsigned int GNC;
        static rwmutex_t gnc_looptime_lock;
        //! Number of milliseconds between Piksi state controller iterations
        static constexpr unsigned int PIKSI = 100;
        //! Number of milliseconds between system output state controller iterations
        static constexpr unsigned int SYSTEM_OUTPUT = 100;
        //! Number of milliseconds between propulsion actuations. This is really high
        // so that the pressure reading has time to stabilize.
        static constexpr unsigned int PROPULSION_ACTUATION_LOOP = 30000;
        //! Number of milliseconds between propulsion repressurization/firing loop iterations
        static constexpr unsigned int PROPULSION_LOOP = 1000;
        //! Number of milliseconds between quake controller iterations
        static constexpr unsigned int QUAKE = 500;
    };

    // ADCS specific
    //! Threads waiting on a finish of ADCS detumbling
    extern threads_queue_t adcs_detumbled;
}

#endif