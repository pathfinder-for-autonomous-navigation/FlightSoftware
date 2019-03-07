#include "../../state/state_holder.hpp"
#include "../controllers.hpp"
#include <array>
#include "../constants.hpp"
#include "orbit_propagators.hpp"

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

    bool is_firing_planned_by_uplink = State::read(State::Propulsion::is_firing_planned_by_uplink, 
                                                        State::Propulsion::propulsion_state_lock);
    if (is_valid_firing && is_firing_planned_by_uplink) {
        State::write(State::Propulsion::is_firing_planned, true, State::Propulsion::propulsion_state_lock);
        State::write(State::Propulsion::firing_data.impulse_vector, firing_vector, State::Propulsion::propulsion_state_lock);
        State::write(State::Propulsion::firing_data.time, firing_time, State::Propulsion::propulsion_state_lock);
    }
}

void RTOSTasks::gnc_controller(void* arg) {
    chRegSetThreadName("GNC");

    GNC::orbit_propagator_thread = chThdCreateStatic(GNC::orbit_propagator_workingArea, 
                                                    sizeof(GNC::orbit_propagator_workingArea),
                                                    GNC::orbit_propagator_thread_priority, 
                                                    GNC::orbit_propagator_controller, NULL);

    systime_t time = chVTGetSystemTimeX();
    while (true) {
        time += MS2ST(RTOSTasks::LoopTimes::GNC);
        gnc_calculation();
        chThdSleepUntil(time);
    }
}