/** @file propulsion.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains implementation for the propulsion state controller.
 */

#include "controllers.hpp"
#include "constants.hpp"
#include "../state/state_holder.hpp"
#include "../deployment_timer.hpp"
#include "../data_collection/data_collection.hpp"
#include <SpikeAndHold/SpikeAndHold.hpp>
#include <tensor.hpp>
#include <AttitudeEstimator.hpp>

namespace RTOSTasks {
    THD_WORKING_AREA(propulsion_controller_workingArea, 2048);
}
namespace Constants {
namespace Propulsion {
    static pla::Vec3f NVECTOR_1; // TODO initializers
    static pla::Vec3f NVECTOR_2;
    static pla::Vec3f NVECTOR_3;
    static pla::Vec3f NVECTOR_4;
    std::map<unsigned char, const pla::Vec3f> NOZZLE_VECTORS = {
        {2, NVECTOR_1},
        {3, NVECTOR_2},
        {4, NVECTOR_3},
        {5, NVECTOR_4}
    };
}
}
using State::Propulsion::PropulsionState;
using State::Propulsion::propulsion_state_lock;
using State::Hardware::spike_and_hold_lock;
using namespace Constants::Propulsion;
using Devices::spike_and_hold;
using Devices::pressure_sensor;
using Devices::temp_sensor_inner;
using Devices::temp_sensor_outer;

static int can_fire_manuever() {
    rwMtxWLock(&propulsion_state_lock);
        State::Propulsion::tank_pressure = pressure_sensor.get();
        State::Propulsion::tank_inner_temperature = temp_sensor_inner.get();
        State::Propulsion::tank_outer_temperature = temp_sensor_outer.get();
    rwMtxWUnlock(&propulsion_state_lock);

    rwMtxRLock(&propulsion_state_lock);
        bool is_propulsion_enabled = State::Propulsion::is_propulsion_enabled;
        bool is_firing_planned = State::Propulsion::is_firing_planned;
        bool is_inner_tank_temperature_too_high = State::Propulsion::tank_pressure >= 100;
        bool is_outer_tank_temperature_too_high = State::Propulsion::tank_inner_temperature >= 48;
        bool is_outer_tank_pressure_too_high = State::Propulsion::tank_outer_temperature >= 48;
    rwMtxRUnlock(&propulsion_state_lock);

    if (!is_propulsion_enabled) return 0;

    if (is_inner_tank_temperature_too_high || is_outer_tank_temperature_too_high || is_outer_tank_pressure_too_high) {
        return -1;
    }
    else if (is_firing_planned) {
        rwMtxRLock(&propulsion_state_lock); 
            gps_time_t firing_time = State::Propulsion::firing_data.time;
        rwMtxRUnlock(&propulsion_state_lock);

        gps_time_t current_time = State::GNC::get_current_time();
        if (current_time > firing_time - Constants::Propulsion::THRUSTER_PREPARATION_TIME) {
            // We cannot execute this firing, since the planned time of the 
            // firing (and its preparation) is less than the current time!
            return 0;
        }

        rwMtxRLock(&State::ADCS::adcs_state_lock);
            bool is_nighttime = !State::ADCS::is_sun_vector_determination_working;
        rwMtxRUnlock(&State::ADCS::adcs_state_lock);
        rwMtxRLock(&State::ADCS::adcs_state_lock);
            bool has_firing_happened_in_nighttime = State::GNC::has_firing_happened_in_nighttime;
        rwMtxRUnlock(&State::ADCS::adcs_state_lock);
        if (is_nighttime && has_firing_happened_in_nighttime) {
            // We cannot execute the firing since we've already done one at night!
            return 0;
        }
        else if (!is_nighttime) {
            // We're now in daylight! So we can set the "has firing happened in nighttime" flag to false
            // since it'll be a while before we return to nighttime.
            has_firing_happened_in_nighttime = false;
        }
        return 1;
    }
    else return 0; // There's no firing to run at all!
}

thread_t* venting_thread;
thread_t* pressurizing_thread;
thread_t* firing_thread;
static THD_WORKING_AREA(venting_thread_wa, 1024);
static THD_WORKING_AREA(pressurizing_thread_wa, 1024);
static THD_WORKING_AREA(firing_thread_wa, 1024);

static THD_FUNCTION(venting_fn, args) {
    debug_println("Tank temperature or pressures are too high. Venting to reduce vapor pressure.");
    // TODO notify ground that overpressure event happened.
    for(int i = 0; i < 10; i++) {
        chMtxLock(&spike_and_hold_lock);
            spike_and_hold.execute_schedule({VALVE_VENT_TIME, VALVE_VENT_TIME, 0, 0, 0, 0});
        chMtxUnlock(&spike_and_hold_lock);
        chThdSleepMilliseconds(VALVE_WAIT_TIME);
    }
}

static void change_propulsion_state(PropulsionState state) {
    rwMtxWLock(&propulsion_state_lock);
        State::Propulsion::propulsion_state = state;
    rwMtxWUnlock(&propulsion_state_lock);
}

static THD_FUNCTION(pressurizing_fn, args) {
    chMtxLock(&spike_and_hold_lock);
    // Pressurize tank
    chMtxUnlock(&spike_and_hold_lock);
    // TODO count pressurizations, check if pressure is starting to match the intended pressure
    // If not, send satellite to safe hold.
}

static THD_FUNCTION(firing_fn, args) {
    rwMtxRLock(&State::ADCS::adcs_state_lock);
        std::array<float, 4> q_body;
        for(int i = 0; i < 4; i++) q_body[i] = ADCSControllers::Estimator::q_filter_body[i];
    rwMtxRUnlock(&State::ADCS::adcs_state_lock);
    pla::Vec3f impulse_vector_body;
    rwMtxRLock(&State::Propulsion::propulsion_state_lock);
        vect_rot(State::Propulsion::firing_data.impulse_vector.data(), q_body.data(), impulse_vector_body.get_data());
    rwMtxRUnlock(&State::Propulsion::propulsion_state_lock);
    std::array<unsigned int, 6> valve_timings;
    valve_timings[0] = 0;
    valve_timings[1] = 0;
    
    // Find three nozzles closest to impulse vector
    unsigned int farthest_nozzle = 2;
    float max_angle = 0;
    for(int i = 2; i < 6; i++) {
        float dot_product = Constants::Propulsion::NOZZLE_VECTORS.at(0) * impulse_vector_body;
        float magnitude_product = Constants::Propulsion::NOZZLE_VECTORS.at(i).length() * impulse_vector_body.length();
        float angle = acos(dot_product / magnitude_product);
        if (angle > max_angle) farthest_nozzle = i;
    }

    pla::Mat3x3f nozzle_vector_matrix; // Matrix of vector nozzles
    int k = 0; // Nozzle counter
    for(int i = 2; i < 6; i++) {
        if (i == farthest_nozzle) continue;
        for (int j = 0; j < 3; j++) nozzle_vector_matrix[k][j] = Constants::Propulsion::NOZZLE_VECTORS.at(i)[j];
        k++;
    }
    pla::Vec3f firing_times = nozzle_vector_matrix.inverse() * impulse_vector_body;
    k = 0;
    for(int i = 2; i < 6; i++) {
        if (i == farthest_nozzle) continue;
        valve_timings[i] = firing_times[k] * 1000; // Convert from seconds to milliseconds
        if (valve_timings[i] > 1000) valve_timings[i] = 1000; // Saturate firing
        k++;
    }

    // Add to delta-v
    rwMtxWLock(&State::Propulsion::propulsion_state_lock);
        State::Propulsion::delta_v_available += 
            vect_mag(State::Propulsion::firing_data.impulse_vector.data()) / Constants::Master::SPACECRAFT_MASS;
    rwMtxWUnlock(&State::Propulsion::propulsion_state_lock);
    chSysLock();
        debug_println("Initiating firing.");
        spike_and_hold.execute_schedule(valve_timings);
        debug_println("Completed firing.");
    chSysUnlock();

    change_propulsion_state(PropulsionState::IDLE);
    chThdExit((msg_t) 0);
}

static void propulsion_state_controller() {
    rwMtxRLock(&propulsion_state_lock);
        PropulsionState propulsion_state = State::Propulsion::propulsion_state;
    rwMtxRUnlock(&propulsion_state_lock);

    int can_manuever = can_fire_manuever();
    switch(propulsion_state) {
        case PropulsionState::IDLE: {
            if (can_manuever == -1) {
                change_propulsion_state(PropulsionState::VENTING);
            }
            else if (can_manuever == 1) {
                change_propulsion_state(PropulsionState::AWAITING_PRESSURIZATION);
            }
        }
        break;
        case PropulsionState::VENTING: {
            if (venting_thread == NULL)
                venting_thread = chThdCreateStatic(venting_thread_wa, sizeof(venting_thread_wa), 
                    RTOSTasks::propulsion_thread_priority, venting_fn, NULL);
        }
        break;
        case PropulsionState::AWAITING_PRESSURIZATION: {
            int can_manuever = can_fire_manuever();
            if (can_manuever == -1)
                change_propulsion_state(PropulsionState::VENTING);
            else if (can_manuever == 0)
                change_propulsion_state(PropulsionState::IDLE);
            else {
                gps_time_t start_pressurization_time = 
                    State::Propulsion::firing_data.time - Constants::Propulsion::THRUSTER_PREPARATION_TIME
                    - 1500; // Buffer time so that the system has time to settle into pressurization
                if (State::GNC::get_current_time() > start_pressurization_time) {
                    change_propulsion_state(PropulsionState::PRESSURIZING);
                }
            }
        }
        break;
        case PropulsionState::PRESSURIZING: {
            if (can_manuever != 1) {
                chThdTerminate(pressurizing_thread);
                pressurizing_thread = NULL;
                if (can_manuever == -1)
                    change_propulsion_state(PropulsionState::VENTING);
                else
                    change_propulsion_state(PropulsionState::IDLE);
            }
            else if (pressurizing_thread == NULL)
                pressurizing_thread = chThdCreateStatic(pressurizing_thread_wa, sizeof(pressurizing_thread_wa), 
                    RTOSTasks::propulsion_thread_priority, pressurizing_fn, NULL);
        }
        break;
        case PropulsionState::FIRING: {
            if (can_manuever != 1) {
                chThdTerminate(firing_thread);
                firing_thread = NULL;
                if (can_manuever == -1)
                    change_propulsion_state(PropulsionState::VENTING);
                else
                    change_propulsion_state(PropulsionState::IDLE);
            }
            else if (State::ADCS::angular_rate() >= State::ADCS::MAX_STABLE_ANGULAR_RATE) {
                // Satellite is too unstable for a firing
                // TODO set some downlink issue
                change_propulsion_state(PropulsionState::IDLE);
            }
            else if (firing_thread == NULL)
                firing_thread = chThdCreateStatic(firing_thread_wa, sizeof(firing_thread_wa), 
                    RTOSTasks::propulsion_thread_priority, firing_fn, NULL);
        }
        break;
        default: {
            // Uh oh, undefined mode; go to idle since that's safest
            change_propulsion_state(PropulsionState::IDLE);
        }
        break;
    }
}

static THD_WORKING_AREA(propulsion_loop_wa, 1024);
static THD_FUNCTION(propulsion_loop, args) {
    systime_t t = chVTGetSystemTime();
    while(true) {
        t += MS2ST(RTOSTasks::LoopTimes::PROPULSION_LOOP);
        propulsion_state_controller();
        chThdSleepUntil(t);
    }
}

void RTOSTasks::propulsion_controller(void *arg) {
    chRegSetThreadName("PROP");
    debug_println("Propulsion controller process has started.");
    chThdCreateStatic(propulsion_loop_wa, sizeof(propulsion_loop_wa), 
        RTOSTasks::propulsion_thread_priority, propulsion_loop, NULL);

    DataCollection::initialize_propulsion_history_timers();

    debug_println("Waiting for deployment timer to finish.");
    rwMtxRLock(&State::Master::master_state_lock);
        bool is_deployed = State::Master::is_deployed;
    rwMtxRUnlock(&State::Master::master_state_lock);
    if (!is_deployed) chThdEnqueueTimeoutS(&deployment_timer_waiting, S2ST(DEPLOYMENT_LENGTH));
    debug_println("Deployment timer has finished.");
    debug_println("Initializing main operation...");

    rwMtxWLock(&propulsion_state_lock);
        State::Propulsion::is_propulsion_enabled = true;
    rwMtxWUnlock(&propulsion_state_lock);

    chThdExit((msg_t)0);
}