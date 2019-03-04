#include "../../state/state_holder.hpp"
#include "../controllers.hpp"
#include <array>
#include "../constants.hpp"

namespace RTOSTasks {
    THD_WORKING_AREA(gnc_controller_workingArea, 2048);
    unsigned int LoopTimes::GNC = 60000;
}

static void gnc_calculation() {
    // TODO run the Matlab-autocoded function to do the calculation
    std::array<float, 3> firing_vector;
    gps_time_t firing_time;

    // Ensure that scheduled firing is bounded properly
    bool is_valid_firing = true;
    if (vect_mag(firing_vector.data()) >= Constants::Propulsion::MAX_FIRING_IMPULSE)
        is_valid_firing = false;
    // Ensure that scheduled firing time is within one orbit
    if ((unsigned int) (firing_time - State::GNC::get_current_time()) > Constants::Master::ORBIT_PERIOD_MS)
        is_valid_firing = false;

    rwMtxRLock(&State::Propulsion::propulsion_state_lock);
        bool is_propulsion_enabled = State::Propulsion::is_propulsion_enabled;
        bool is_firing_planned_by_uplink = State::Propulsion::is_firing_planned_by_uplink;
    rwMtxRUnlock(&State::Propulsion::propulsion_state_lock);
    if (is_propulsion_enabled && is_valid_firing && is_firing_planned_by_uplink) {
        rwMtxWLock(&State::Propulsion::propulsion_state_lock);
            State::Propulsion::is_firing_planned = true;
            State::Propulsion::firing_data.impulse_vector = firing_vector;
            State::Propulsion::firing_data.time = firing_time;
        rwMtxWUnlock(&State::Propulsion::propulsion_state_lock);
    }
}

void RTOSTasks::gnc_controller(void* arg) {
    systime_t time = chVTGetSystemTimeX();
    while (true) {
        time += MS2ST(RTOSTasks::LoopTimes::GNC);
        gnc_calculation();
        chThdSleepUntil(time);
    }
}