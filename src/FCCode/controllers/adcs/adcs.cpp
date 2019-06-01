/** @file controllers/adcs.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains implementation for the ADCS state controller.
 */

#include "../controllers.hpp"
#include "../constants.hpp"
#include <ADCSEnums.hpp>
#include <rwmutex.hpp>
#include "adcs_helpers.hpp"
#include "../../state/state_holder.hpp"
#include "../gomspace/power_cyclers.hpp"
#include "../../deployment_timer.hpp"
#include "../../data_collection/data_collection.hpp"
#include <AttitudePDcontrol.hpp>
#include <AttitudeEstimator.hpp>
#include <AttitudeMath.hpp>
#include <tensor.hpp>
#include <MomentumControl.hpp>

namespace RTOSTasks {
    THD_WORKING_AREA(adcs_controller_workingArea, 2048);
    threads_queue_t adcs_detumbled;
}
using State::ADCS::ADCSState;
using State::ADCS::adcs_state_lock;
using Devices::adcs_system;

using namespace ADCSControllers;

static void update_gain_constants() {
    ADCSControllers::AttitudePD::kd = (float) Constants::read(Constants::ADCS::ATTITUDE_CONTROLLER_KD);
    ADCSControllers::AttitudePD::kp = (float) Constants::read(Constants::ADCS::ATTITUDE_CONTROLLER_KP);
    ADCSControllers::MomentumControl::k = (float) Constants::read(Constants::ADCS::MOMENTUM_CONTROLLER_K);
    // TODO add gyro heater gains
}

static THD_WORKING_AREA(adcs_loop_workingArea, 4096);
static THD_FUNCTION(adcs_loop, arg) {
    chRegSetThreadName("adcs.loop");

    systime_t t0 = chVTGetSystemTimeX();
    while(true) {
        t0 += S2ST(1000);

        // Read from ADCS system for data
        ADCSControllers::read_adcs_data();
        // If gain constants were modified by uplink, change them in the estimator
        update_gain_constants();
        // Run the estimator
        ADCSControllers::Estimator::update();

        // If ADCS isn't working, power-cycle it. Do this for as many times as it takes for the device
        // to start talking again.
        if (!State::Hardware::check_is_functional(adcs_system) && Gomspace::adcs_system_thread == NULL) {
            // Specify arguments for thread
            Gomspace::cycler_arg_t cycler_args = {
                &State::Hardware::adcs_device_lock,
                Devices::adcs_system,
                Devices::Gomspace::DEVICE_PINS::ADCS
            };
            // Start cycler thread
            Gomspace::adcs_system_thread = chThdCreateFromMemoryPool(&Gomspace::power_cycler_pool,
                "PCYCLER:ADCS",
                RTOSTasks::master_thread_priority,
                Gomspace::cycler_fn, (void*) &cycler_args);
        }

        // State machine for ADCS operation
        rwMtxRLock(&State::ADCS::adcs_state_lock);
            bool adcs_control_allowed = (State::ADCS::adcs_state != State::ADCS::ADCS_SAFE_HOLD);
        rwMtxRUnlock(&State::ADCS::adcs_state_lock);
        if (adcs_control_allowed) {
            rwMtxRLock(&adcs_state_lock);
                ADCSState adcs_state = State::ADCS::adcs_state;
            rwMtxRUnlock(&adcs_state_lock);
            switch(adcs_state) {
                case ADCSState::ADCS_DETUMBLE: {
                    chMtxLock(&State::Hardware::adcs_device_lock);
                        if (!State::Hardware::check_is_functional(Devices::adcs_system))
                            adcs_system->set_mode(Mode::ACTIVE);
                    chMtxLock(&State::Hardware::adcs_device_lock);
                    if (State::ADCS::angular_rate() < Constants::ADCS::MAX_STABLE_ANGULAR_RATE) {
                        chThdDequeueAllI(&RTOSTasks::adcs_detumbled, (msg_t) 0);
                        rwMtxWLock(&adcs_state_lock);
                            State::ADCS::adcs_state = ADCSState::ZERO_TORQUE;
                        rwMtxWUnlock(&adcs_state_lock);
                        break;
                    }
                    MomentumControl::magfield = ADCSControllers::Estimator::magfield_filter_body;
                    MomentumControl::momentum = ADCSControllers::Estimator::htotal_filter_body;
                    MomentumControl::update();
                    State::write(State::ADCS::mtr_cmds, MomentumControl::moment, adcs_state_lock);
                    rwMtxRLock(&adcs_state_lock);
                        std::array<float, 3> mtr_cmds = State::ADCS::mtr_cmds;
                        adcs_system->set_mtr_cmd(mtr_cmds.data());
                    rwMtxRUnlock(&adcs_state_lock);
                }
                break;
                case ADCSState::ZERO_TORQUE: {
                    chMtxLock(&State::Hardware::adcs_device_lock);
                        if (!State::Hardware::check_is_functional(adcs_system))
                            adcs_system->set_mode(Mode::ACTIVE);
                    chMtxLock(&State::Hardware::adcs_device_lock);
                    // Set MTR to zero in state
                    rwMtxWLock(&adcs_state_lock);
                        for(int i = 0; i < 3; i++) State::ADCS::mtr_cmds[i] = 0.0f;
                    rwMtxWUnlock(&adcs_state_lock);
                    // Command constant wheel speed and MTR
                    std::array<float, 3> mtr_cmds, rwa_speed_cmds;
                    rwMtxRLock(&adcs_state_lock);
                        mtr_cmds.fill(0);
                        rwa_speed_cmds = State::ADCS::rwa_speed_cmds;
                    rwMtxRUnlock(&adcs_state_lock);
                    chMtxLock(&State::Hardware::adcs_device_lock);
                        if (!State::Hardware::check_is_functional(adcs_system)) {
                            adcs_system->set_mtr_cmd(mtr_cmds.data());
                            adcs_system->set_rwa_mode(RWAMode::SPEED_CTRL, rwa_speed_cmds.data());
                        }
                    chMtxUnlock(&State::Hardware::adcs_device_lock);
                }
                break;
                case ADCSState::POINTING: {
                    chMtxLock(&State::Hardware::adcs_device_lock);
                        if (!State::Hardware::check_is_functional(adcs_system))
                            adcs_system->set_mode(Mode::ACTIVE);
                    chMtxLock(&State::Hardware::adcs_device_lock);

                    rwMtxRLock(&adcs_state_lock);
                        // Adjust commanded attitude to be in ECI frame
                        std::array<float, 4> cmd_attitude_eci;
                        ADCSControllers::get_command_attitude_in_eci(&cmd_attitude_eci);
                        // Convert command attitude to delta quaternion and feed data to controller
                        quat_rot_diff(cmd_attitude_eci.data(), State::ADCS::cur_attitude.data(), AttitudePD::deltaquat.data());
                        AttitudePD::angrate = State::ADCS::cur_ang_rate;
                    rwMtxRUnlock(&adcs_state_lock);

                    AttitudePD::update();
                    State::write(State::ADCS::rwa_torques, AttitudePD::torque, adcs_state_lock);
                    chMtxLock(&State::Hardware::adcs_device_lock);
                        if (!State::Hardware::check_is_functional(Devices::adcs_system))
                            adcs_system->set_rwa_mode(RWAMode::ACCEL_CTRL, AttitudePD::torque.data());
                    chMtxUnlock(&State::Hardware::adcs_device_lock);
                }
                break;
                case ADCSState::ADCS_SAFE_HOLD: {
                    adcs_system->set_mode(Mode::PASSIVE);
                }
                break;
                default: {
                    rwMtxRLock(&adcs_state_lock);
                        State::ADCS::adcs_state = ADCSState::ADCS_SAFE_HOLD;
                    rwMtxRUnlock(&adcs_state_lock);
                }
            }
        }
        chThdSleepUntil(t0);
    }
}

static THD_WORKING_AREA(update_adcs_hat_workingArea, 1024);
static THD_FUNCTION(update_adcs_hat, args) {
    chRegSetThreadName("adcs.update_hat");
    systime_t t = chVTGetSystemTimeX();
    while(true) {
        t += MS2ST(RTOSTasks::LoopTimes::ADCS_HAT_CHECK);

        chMtxLock(&State::Hardware::adcs_device_lock);
            if (State::Hardware::check_is_functional(adcs_system)) 
                adcs_system->update_hat(); // TODO fix
        chMtxUnlock(&State::Hardware::adcs_device_lock);

        chThdSleepUntil(t);
    }
}

void RTOSTasks::adcs_controller(void *arg) {
    chRegSetThreadName("adcs");
    dbg.println("ADCS controller process has started.");
    
    chThdCreateStatic(adcs_loop_workingArea, sizeof(adcs_loop_workingArea), 
        RTOSTasks::adcs_thread_priority, adcs_loop, NULL);

    DataCollection::initialize_adcs_history_timers();

    // Create HAT updater thread
    chThdCreateStatic(update_adcs_hat_workingArea,
        sizeof(update_adcs_hat_workingArea), RTOSTasks::adcs_thread_priority, update_adcs_hat, NULL);

    dbg.println("Waiting for deployment timer to finish.");
    rwMtxRLock(&State::Master::master_state_lock);
        bool is_deployed = State::Master::is_deployed;
    rwMtxRUnlock(&State::Master::master_state_lock);
    if (!is_deployed) chThdEnqueueTimeoutS(&deployment_timer_waiting, S2ST(DEPLOYMENT_LENGTH));
    dbg.println("Deployment timer has finished.");
    
    dbg.println("Initializing main operation...");
    chMtxLock(&State::Hardware::adcs_device_lock);
        if (State::Hardware::check_is_functional(adcs_system))
            adcs_system->set_mode(Mode::ACTIVE);
        // TODO what if there's an else?
    chMtxUnlock(&State::Hardware::adcs_device_lock);

    chThdExit((msg_t)0);
}