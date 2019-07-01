#ifndef ORBIT_PROPAGATORS_HPP_
#define ORBIT_PROPAGATORS_HPP_

#include <ChRt.h>
#include "../controllers.hpp"

namespace GNC {
    //! Working area for orbit propagator process
    extern THD_WORKING_AREA(orbit_propagator_workingArea, 2048);
    //! Pointer to the orbit propagator's thread object
    extern thread_t* orbit_propagator_thread;
    //! Orbit propagator controller process
    extern THD_FUNCTION(orbit_propagator_controller, arg);
    //! Priority of GNC controller's thread
    constexpr tprio_t orbit_propagator_thread_priority = RTOSTasks::MAX_THREAD_PRIORITY;
    //! Milliseconds between orbit propagator repetitions
    constexpr unsigned int ORBIT_PROPAGATOR_DELTA_T = 100;
}

#endif