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

namespace RTOSTasks {
    THD_WORKING_AREA(propulsion_controller_workingArea, 4096);
}
using State::Propulsion::propulsion_state_lock;
using State::Hardware::spike_and_hold_lock;
using namespace Constants::Propulsion;
using Devices::spike_and_hold;
using Devices::pressure_sensor;
using Devices::temp_sensor_inner;
using Devices::temp_sensor_outer;

static virtual_timer_t disable_pressurization_timer;
static virtual_timer_t propulsion_firing_timer;

/**
 * @brief Disables the repressurization of tank 2.
 */
void disable_pressurization(void* args) {
    rwMtxWLock(&propulsion_state_lock);
        State::Propulsion::is_repressurization_active = false;
    rwMtxWUnlock(&propulsion_state_lock);
}

/**
 * @brief Disables/cancels a scheduled thruster firing, and also
 * disables repressurization.
 */
void disable_thruster_firing() {
    rwMtxWLock(&propulsion_state_lock);
        State::Propulsion::is_firing_planned = false;
        State::Propulsion::is_repressurization_active = false;
        State::ADCS::is_propulsion_pointing_active = false;
    rwMtxWUnlock(&propulsion_state_lock);
    chVTDoResetI(&propulsion_firing_timer);
}

static void fire_thrusters(void* args) {
    if (State::ADCS::angular_rate() >= State::ADCS::MAX_STABLE_ANGULAR_RATE) {
        // Satellite is too unstable for a firing
        disable_thruster_firing();
        return;
    }

    // TODO compute tank thrusts based on DCM defined by current position
    rwMtxRLock(&State::Piksi::piksi_state_lock);
        std::array<double, 3> position = State::Piksi::gps_position;
    rwMtxRUnlock(&State::Piksi::piksi_state_lock);

    // Add to delta-v
    pla::Vec3d thrust_vec;
    rwMtxRLock(&State::Propulsion::propulsion_state_lock);
        for(int i = 0; i < 2; i++) thrust_vec[i] = State::Propulsion::firing_data.thrust_vector[i];
    rwMtxRLock(&State::Propulsion::propulsion_state_lock);
    rwMtxWLock(&State::Propulsion::propulsion_state_lock);
        State::Propulsion::delta_v_available += thrust_vec.length() / Constants::Master::SPACECRAFT_MASS;
    rwMtxWUnlock(&State::Propulsion::propulsion_state_lock);
    chSysLockFromISR();
        debug_println("Initiating firing.");
        // TODO actually fire from tanks
        debug_println("Completed firing.");
    chSysUnlockFromISR();
    
    disable_thruster_firing();
}

static void prepare_thruster_firing() {
    rwMtxRLock(&propulsion_state_lock);
        gps_time_t firing_time = State::Propulsion::firing_data.thrust_time;
    rwMtxRUnlock(&propulsion_state_lock);

    rwMtxRLock(&State::Piksi::piksi_state_lock);
    gps_time_t current_time = State::Piksi::current_time;
    rwMtxRLock(&State::Piksi::piksi_state_lock);
    if (current_time > firing_time) {
        // We cannot execute this firing, since the planned time of the firing is less than the current time!
        disable_thruster_firing();
        return;
    }
        
    unsigned int firing_time_delta = (unsigned int) (firing_time - current_time);
    if (firing_time_delta < Constants::Propulsion::THRUSTER_PREPARATION_TIME) {
        rwMtxWLock(&propulsion_state_lock);
            State::Propulsion::is_repressurization_active = true;
        rwMtxWUnlock(&propulsion_state_lock);

        rwMtxRLock(&State::Gomspace::gomspace_state_lock);
            unsigned short int battlevel = State::Gomspace::gomspace_data.vbatt;
        rwMtxRUnlock(&State::Gomspace::gomspace_state_lock);
        rwMtxRLock(&propulsion_state_lock);
            unsigned short int delta_v_available = State::Propulsion::delta_v_available;
        rwMtxRUnlock(&propulsion_state_lock);
        if (battlevel > Constants::Propulsion::REORIENTATION_BATT_THRESHOLD || delta_v_available < Constants::Propulsion::REORIENTATION_DELTA_V_THRESHOLD) {
            debug_println("Orienting in a more desirable attitude for thruster firing.");
            rwMtxWLock(&State::ADCS::adcs_state_lock);
                State::ADCS::is_propulsion_pointing_active = true;
                State::ADCS::adcs_state = State::ADCS::ADCSState::POINTING;
            rwMtxWUnlock(&State::ADCS::adcs_state_lock);
            // TODO temporarily set attitude in desired orientation
        }
        chSysLock();
            chVTSet(&disable_pressurization_timer, MS2ST(firing_time_delta - Constants::Propulsion::STOP_PRESSURIZATION_TIME_DELTA), disable_pressurization, NULL);
            chVTSet(&propulsion_firing_timer, MS2ST(firing_time_delta), fire_thrusters, NULL);
        chSysUnlock();
    }
}

static THD_WORKING_AREA(propulsion_actuation_loop_workingArea, 4096);
static THD_FUNCTION(propulsion_actuation_loop, arg) {
    systime_t t0 = chVTGetSystemTimeX();
    while(true) {
        t0 += S2ST(RTOSTasks::LoopTimes::PROPULSION_ACTUATION_LOOP);
        rwMtxRLock(&propulsion_state_lock);
            bool is_firing_planned = State::Propulsion::is_firing_planned;
            bool is_repressurization_active = State::Propulsion::is_repressurization_active;
            bool is_inner_tank_temperature_too_high = State::Propulsion::tank_pressure >= 100;
            bool is_outer_tank_temperature_too_high = State::Propulsion::tank_inner_temperature >= 48;
            bool is_outer_tank_pressure_too_high = State::Propulsion::tank_outer_temperature >= 48;
        rwMtxRUnlock(&propulsion_state_lock);

        if (is_inner_tank_temperature_too_high) {
            debug_println("Inner tank temperature is too high. Venting to reduce vapor pressure.");
            // TODO notify ground that overpressure event happened.
            disable_thruster_firing();
            for(int i = 0; i < 10; i++) {
                chMtxLock(&spike_and_hold_lock);
                    spike_and_hold.execute_schedule({VALVE_VENT_TIME, VALVE_VENT_TIME, 0, 0, 0, 0});
                chMtxUnlock(&spike_and_hold_lock);
                chThdSleepMilliseconds(VALVE_WAIT_TIME);
            }  
        }
        else if (is_outer_tank_temperature_too_high || is_outer_tank_pressure_too_high) {
            debug_println("Outer tank temperature or pressure is too high. Venting to reduce pressure.");
            // TODO notify ground that overpressure event happened.
            disable_thruster_firing();
            chMtxLock(&spike_and_hold_lock);
                for(int i = 0; i < 10; i++) {
                    chMtxLock(&spike_and_hold_lock);
                        spike_and_hold.execute_schedule({VALVE_VENT_TIME, VALVE_VENT_TIME, 0, 0, 0, 0});
                    chMtxUnlock(&spike_and_hold_lock);
                    chThdSleepMilliseconds(VALVE_WAIT_TIME);
                }
            chMtxUnlock(&spike_and_hold_lock);
        }
        else if (is_firing_planned && !chVTIsArmedI(&propulsion_firing_timer)) {
            prepare_thruster_firing();
        }
        else if (is_repressurization_active) {
            chMtxLock(&spike_and_hold_lock);
            // Pressurize tank
            chMtxUnlock(&spike_and_hold_lock);
            // TODO count pressurizations, check if pressure is starting to match the intended pressure
            // If not, stop allowing prop firings to be scheduled by doing the following:
            //
            // chVTReset(&propulsion_firing_timer);
            // rwMtxWLock(&propulsion_state_lock);
            //     State::Propulsion::is_propulsion_enabled = false;
            // rwMtxWUnlock(&propulsion_state_lock);
        }
        chThdSleepUntil(t0);
    }
}

static THD_WORKING_AREA(propulsion_loop_workingArea, 4096);
static THD_FUNCTION(propulsion_loop, arg) {
    chRegSetThreadName("PROP LOOP");
    systime_t t0 = chVTGetSystemTimeX();
    while(true) {
        t0 += S2ST(RTOSTasks::LoopTimes::PROPULSION_LOOP);
        // Read pressure values
        rwMtxWLock(&propulsion_state_lock);
            State::Propulsion::tank_pressure = pressure_sensor.get();
            State::Propulsion::tank_inner_temperature = temp_sensor_inner.get();
            State::Propulsion::tank_outer_temperature = temp_sensor_outer.get();
        rwMtxWUnlock(&propulsion_state_lock);
        chThdSleepUntil(t0);
    }
}

void RTOSTasks::propulsion_controller(void *arg) {
    chRegSetThreadName("PROP");
    debug_println("Propulsion controller process has started.");
    chThdCreateStatic(propulsion_loop_workingArea, sizeof(propulsion_loop_workingArea), 
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
        State::Propulsion::is_repressurization_active = false;
    rwMtxWUnlock(&propulsion_state_lock);
    chThdCreateStatic(propulsion_actuation_loop_workingArea, sizeof(propulsion_actuation_loop_workingArea), 
        RTOSTasks::propulsion_thread_priority, propulsion_actuation_loop, NULL);

    chThdExit((msg_t)0);
}