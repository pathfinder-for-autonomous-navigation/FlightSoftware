/** @file propulsion.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains implementation for the propulsion state controller.
 */

#include <EEPROM.h>
#include "../controllers.hpp"
#include "../../state/EEPROMAddresses.hpp"
#include "../../state/fault_state_holder.hpp"
#include "../../state/state_holder.hpp"
#include "../../deployment_timer.hpp"
#include "../../data_collection/data_collection.hpp"
#include "propulsion_tasks.hpp"
#include <SpikeAndHold.hpp>

namespace RTOSTasks {
    THD_WORKING_AREA(propulsion_controller_workingArea, 2048);
}
using State::Propulsion::PropulsionState;
using State::Propulsion::propulsion_state_lock;

namespace PropulsionTasks {
    rwmutex_t propulsion_thread_ptr_lock;
}

using namespace PropulsionTasks;

static void propulsion_state_controller() {
    PropulsionState propulsion_state = State::read(State::Propulsion::propulsion_state, propulsion_state_lock);

    int can_manuever = can_fire_manuever();
    switch(propulsion_state) {
        case PropulsionState::DISABLED:
            // Actually do nothing except collect data, which is already
            // being collected by can_fire_manuever()
        break;
        case PropulsionState::IDLE: {
            if (can_manuever == -1)
                State::write(State::Propulsion::propulsion_state, PropulsionState::VENTING, propulsion_state_lock);
            else if (can_manuever == 1)
                State::write(State::Propulsion::propulsion_state, PropulsionState::AWAITING_PRESSURIZATION, propulsion_state_lock);
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
                State::write(State::Propulsion::propulsion_state, PropulsionState::VENTING, propulsion_state_lock);
            else if (can_manuever == 0)
                State::write(State::Propulsion::propulsion_state, PropulsionState::IDLE, propulsion_state_lock);
            else {
                unsigned int stop_pressurization_time_delta = Constants::read(Constants::Propulsion::STOP_PRESSURIZATION_TIME_DELTA);
                gps_time_t start_pressurization_time = State::Propulsion::firing_data.time - stop_pressurization_time_delta;
                if (State::GNC::get_current_time() > start_pressurization_time)
                    State::write(State::Propulsion::propulsion_state, PropulsionState::PRESSURIZING, propulsion_state_lock);
            }
        }
        break;
        case PropulsionState::PRESSURIZING: {
            if (can_manuever != 1) {
                chThdTerminate(pressurizing_thread);
                if (can_manuever == -1)
                    State::write(State::Propulsion::propulsion_state, PropulsionState::VENTING, propulsion_state_lock);
                else
                    State::write(State::Propulsion::propulsion_state, PropulsionState::IDLE, propulsion_state_lock);
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
                    State::write(State::Propulsion::propulsion_state, PropulsionState::VENTING, propulsion_state_lock);
                else
                    State::write(State::Propulsion::propulsion_state, PropulsionState::IDLE, propulsion_state_lock);
            }
            if (State::ADCS::angular_rate() >= Constants::ADCS::MAX_STABLE_ANGULAR_RATE) {
                // Satellite is too unstable for a firing
                State::write(FaultState::Propulsion::destabilization_event, 
                    State::GNC::get_current_time(), 
                    FaultState::Propulsion::propulsion_faults_state_lock);
                State::write(State::Propulsion::propulsion_state, PropulsionState::IDLE, propulsion_state_lock);
            }
            float tank_pressure = State::read(State::Propulsion::tank_pressure, propulsion_state_lock);
            if (tank_pressure < Constants::Propulsion::PRE_FIRING_OUTER_TANK_PRESSURE) {
                // Not enough pressure for a firing
                // TODO record fault. Why did pressure leak so fast?
                State::write(State::Propulsion::propulsion_state, PropulsionState::IDLE, propulsion_state_lock);
            }

            if (firing_thread == NULL || chThdTerminatedX(firing_thread))
                firing_thread = chThdCreateStatic(firing_thread_wa, sizeof(firing_thread_wa), 
                    RTOSTasks::propulsion_thread_priority, firing_fn, NULL);
        }
        break;
        default: {
            // Uh oh, undefined mode; go to disabled mode since that's safest
            State::write(State::Propulsion::propulsion_state, PropulsionState::DISABLED, propulsion_state_lock);
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
        unsigned char preferred_valve = EEPROM.read(EEPROM_ADDRESSES::PREFERRED_INTERTANK_VALVE);
    chMtxUnlock(&eeprom_lock);
    State::write(State::Propulsion::intertank_firing_valve, preferred_valve, propulsion_state_lock);

    debug_println("Waiting for deployment timer to finish.");
    bool is_deployed = State::read(State::Master::is_deployed, State::Master::master_state_lock);
    if (!is_deployed) chThdEnqueueTimeoutS(&deployment_timer_waiting, S2ST(DEPLOYMENT_LENGTH));
    debug_println("Deployment timer has finished.");
    debug_println("Initializing main operation...");
    State::write(State::Propulsion::propulsion_state, PropulsionState::IDLE, propulsion_state_lock);

    chThdExit((msg_t)0);
}