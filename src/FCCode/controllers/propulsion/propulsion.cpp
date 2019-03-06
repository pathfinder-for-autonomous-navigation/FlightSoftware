/** @file propulsion.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains implementation for the propulsion state controller.
 */

#include <EEPROM.h>
#include "../controllers.hpp"
#include "../../state/EEPROMAddresses.hpp"
#include "../../state/state_holder.hpp"
#include "../../deployment_timer.hpp"
#include "../../data_collection/data_collection.hpp"
#include "propulsion_tasks.hpp"
#include <SpikeAndHold/SpikeAndHold.hpp>

namespace RTOSTasks {
    THD_WORKING_AREA(propulsion_controller_workingArea, 2048);
}
using State::Propulsion::PropulsionState;
using State::Propulsion::propulsion_state_lock;
using Devices::pressure_sensor;
using Devices::temp_sensor_inner;
using Devices::temp_sensor_outer;

namespace PropulsionTasks {
    void change_propulsion_state(PropulsionState state) {
        rwMtxWLock(&propulsion_state_lock);
            State::Propulsion::propulsion_state = state;
        rwMtxWUnlock(&propulsion_state_lock);
    }
    rwmutex_t propulsion_thread_ptr_lock;
}

using namespace PropulsionTasks;

static int can_fire_manuever() {
    rwMtxWLock(&propulsion_state_lock);
        State::Propulsion::tank_pressure = pressure_sensor.get();
        State::Propulsion::tank_inner_temperature = temp_sensor_inner.get();
        State::Propulsion::tank_outer_temperature = temp_sensor_outer.get();
    rwMtxWUnlock(&propulsion_state_lock);

    rwMtxRLock(&propulsion_state_lock);
        bool is_firing_planned = State::Propulsion::is_firing_planned;
        bool is_inner_tank_temperature_too_high = State::Propulsion::tank_pressure >= 100;
        bool is_outer_tank_temperature_too_high = State::Propulsion::tank_inner_temperature >= 48;
        bool is_outer_tank_pressure_too_high = State::Propulsion::tank_outer_temperature >= 48;
    rwMtxRUnlock(&propulsion_state_lock);

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

static void propulsion_state_controller() {
    rwMtxRLock(&propulsion_state_lock);
        PropulsionState propulsion_state = State::Propulsion::propulsion_state;
    rwMtxRUnlock(&propulsion_state_lock);

    int can_manuever = can_fire_manuever();
    switch(propulsion_state) {
        case PropulsionState::DISABLED:
            // Actually do nothing except collect data, which is already
            // being collected by can_fire_manuever()
        break;
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
            if (venting_thread == NULL || chThdTerminatedX(venting_thread))
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
                if (can_manuever == -1)
                    change_propulsion_state(PropulsionState::VENTING);
                else
                    change_propulsion_state(PropulsionState::IDLE);
            }
            else if (pressurizing_thread == NULL || chThdTerminatedX(pressurizing_thread))
                pressurizing_thread = chThdCreateStatic(pressurizing_thread_wa, sizeof(pressurizing_thread_wa), 
                    RTOSTasks::propulsion_thread_priority, pressurizing_fn, NULL);
        }
        break;
        case PropulsionState::FIRING: {
            if (can_manuever != 1) {
                chThdTerminate(firing_thread);
                if (can_manuever == -1)
                    change_propulsion_state(PropulsionState::VENTING);
                else
                    change_propulsion_state(PropulsionState::IDLE);
            }
            if (State::ADCS::angular_rate() >= Constants::ADCS::MAX_STABLE_ANGULAR_RATE) {
                // Satellite is too unstable for a firing
                // TODO set some downlink issue
                change_propulsion_state(PropulsionState::IDLE);
            }
            float tank_pressure;
            rwMtxRLock(&propulsion_state_lock);
                tank_pressure = State::Propulsion::tank_pressure;
            rwMtxRUnlock(&propulsion_state_lock);
            if (tank_pressure < Constants::Propulsion::PRE_FIRING_OUTER_TANK_PRESSURE) {
                // Not enough pressure for a firing
                change_propulsion_state(PropulsionState::IDLE);
            }

            if (firing_thread == NULL || chThdTerminatedX(firing_thread))
                firing_thread = chThdCreateStatic(firing_thread_wa, sizeof(firing_thread_wa), 
                    RTOSTasks::propulsion_thread_priority, firing_fn, NULL);
        }
        break;
        default: {
            // Uh oh, undefined mode; go to idle since that's safest
            change_propulsion_state(PropulsionState::DISABLED);
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

    rwMtxObjectInit(&PropulsionTasks::propulsion_thread_ptr_lock);
    DataCollection::initialize_propulsion_history_timers();

    chMtxLock(&eeprom_lock);
        char preferred_valve = EEPROM.read(EEPROM_ADDRESSES::PREFERRED_INTERTANK_VALVE);
    chMtxUnlock(&eeprom_lock);
    rwMtxWLock(&propulsion_state_lock);
        State::Propulsion::intertank_firing_valve = preferred_valve;
    rwMtxWUnlock(&propulsion_state_lock);

    debug_println("Waiting for deployment timer to finish.");
    rwMtxRLock(&State::Master::master_state_lock);
        bool is_deployed = State::Master::is_deployed;
    rwMtxRUnlock(&State::Master::master_state_lock);
    if (!is_deployed) chThdEnqueueTimeoutS(&deployment_timer_waiting, S2ST(DEPLOYMENT_LENGTH));
    debug_println("Deployment timer has finished.");
    debug_println("Initializing main operation...");

    chThdExit((msg_t)0);
}