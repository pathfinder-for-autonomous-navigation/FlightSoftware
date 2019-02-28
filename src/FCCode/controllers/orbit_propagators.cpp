#include "../state/state_holder.hpp"
#include "controllers.hpp"

namespace RTOSTasks {
    THD_WORKING_AREA(obp_controller_workingArea, 4096);
}

static void propagate_self_orbit() {
    // TODO add check for if we're leaving nighttime, and if so, change the has_firing_happened_in_nighttime to false
}

static void propagate_other_orbit() {

}

void orbit_propagator_controller(void* args) {
    systime_t time = chVTGetSystemTimeX();
    while (true) {
        time += MS2ST(RTOSTasks::LoopTimes::ORBIT_PROPAGATOR);
        propagate_self_orbit();
        propagate_other_orbit();
        chThdSleepUntil(time);
    }
}