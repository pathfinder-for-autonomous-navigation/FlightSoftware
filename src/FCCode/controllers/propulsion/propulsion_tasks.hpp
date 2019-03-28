#ifndef PROPULSION_THREADS_HPP_
#define PROPULSION_THREADS_HPP_

#include <ChRt.h>
#include "../controllers.hpp"
#include "../../state/state_holder.hpp"
#include "../constants.hpp"

namespace PropulsionTasks {
    extern thread_t* pressurizing_thread;
    extern thread_t* firing_thread;
    extern thread_t* venting_thread;
    extern rwmutex_t propulsion_thread_ptr_lock;

    int can_fire_manuever();

    extern THD_WORKING_AREA(pressurizing_thread_wa, 1024);
    extern THD_FUNCTION(pressurizing_fn, args);
    
    extern THD_WORKING_AREA(firing_thread_wa, 1024);
    extern THD_FUNCTION(firing_fn, args);
    
    extern THD_WORKING_AREA(venting_thread_wa, 1024);
    extern THD_FUNCTION(venting_fn, args);
}

#endif