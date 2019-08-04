#include <array>
#include "../../state/state_holder.hpp"
#include "../constants.hpp"
#include "../controllers.hpp"
#include "orbit_propagators.hpp"

namespace RTOSTasks {
THD_WORKING_AREA(gnc_controller_workingArea, 2048);
unsigned int LoopTimes::GNC = 60000;
rwmutex_t LoopTimes::gnc_looptime_lock;
}  // namespace RTOSTasks

using State::Propulsion::propulsion_state_lock;

static void gnc_calculation() {
    // TODO run the Matlab-autocoded function to do the calculation
    std::array<float, 3> firing_vector;
    gps_time_t firing_time;

    // Ensure that scheduled firing is bounded properly
    bool is_valid_firing = true;
    if (vect_mag(firing_vector.data()) >= Constants::Propulsion::MAX_FIRING_IMPULSE)
        is_valid_firing = false;
    // Ensure that scheduled firing time is within one orbit
    if ((unsigned int)(firing_time - State::GNC::get_current_time()) >
        Constants::Master::ORBIT_PERIOD_MS)
        is_valid_firing = false;

    State::Master::PANState pan_state =
        State::read(State::Master::pan_state, State::Master::master_state_lock);
    bool is_not_standby = pan_state != State::Master::PANState::STANDBY;
    if (is_valid_firing && is_not_standby) {
        dbg.println(debug_severity::INFO, "Writing recommended firing to state.");
        State::write(State::Propulsion::firing_data.impulse_vector, firing_vector,
                     propulsion_state_lock);
        State::write(State::Propulsion::firing_data.time, firing_time, propulsion_state_lock);
    } else {
        dbg.println(debug_severity::NOTICE, "No firing recommended.");
    }
}

void RTOSTasks::gnc_controller(void *arg) {
    chRegSetThreadName("gnc");

    GNC::orbit_propagator_thread = chThdCreateStatic(
        GNC::orbit_propagator_workingArea, sizeof(GNC::orbit_propagator_workingArea),
        GNC::orbit_propagator_thread_priority, GNC::orbit_propagator_controller, NULL);

    systime_t time = chVTGetSystemTimeX();
    while (true) {
        time += MS2ST(RTOSTasks::LoopTimes::GNC);
        dbg.println(debug_severity::INFO, "Running GNC calculation.");
        gnc_calculation();
        chThdSleepUntil(time);
    }
}