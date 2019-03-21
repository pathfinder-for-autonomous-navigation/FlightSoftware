/** @file controllers/gomspace.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains implementation for the gomspace state controller.
 */

#include "../controllers.hpp"
#include "../constants.hpp"
#include "../../state/state_holder.hpp"
#include "../../state/fault_state_holder.hpp"
#include "../../deployment_timer.hpp"

using Devices::Gomspace;
using Devices::gomspace;
using State::Gomspace::gomspace_data;
using State::Gomspace::gomspace_state_lock;
using FaultState::Gomspace::GOMSPACE_FAULTS;

namespace RTOSTasks {
    THD_WORKING_AREA(gomspace_controller_workingArea, 2048);
}

static void gomspace_read() {
    debug_printf("Reading Gomspace data...");
    unsigned char t = 0; // # of tries at reading housekeeping data
    while (t < 5) {
        bool successful_response = false;
        if (State::Hardware::check_is_functional(&gomspace())) {
            chMtxLock(&State::Hardware::gomspace_device_lock);
                rwMtxWLock(&State::Gomspace::gomspace_state_lock);
                    successful_response = gomspace().get_hk();
                rwMtxWUnlock(&State::Gomspace::gomspace_state_lock);
            chMtxUnlock(&State::Hardware::gomspace_device_lock);
        }
        if (successful_response) break;
        t++;
    }
    if (t == 5) {
        debug_println("unable to read Gomspace data.");
        State::write((State::Hardware::hat).at(&gomspace()).is_functional, 
                            false, State::Hardware::hardware_state_lock);
    }
    else debug_printf("battery voltage (mV): %d\n", gomspace_data.vbatt);
}

static void set_error(GOMSPACE_FAULTS fault, bool value) {
    rwMtxWLock(&FaultState::Gomspace::gomspace_faults_state_lock);
        FaultState::Gomspace::fault_bits.set(fault, value);
    rwMtxWUnlock(&FaultState::Gomspace::gomspace_faults_state_lock);
}

static void set_hardware_error(const Devices::Device* dev_name, const std::string field, bool value) {
    GOMSPACE_FAULTS group;
    if (dev_name == &Devices::piksi()) group = GOMSPACE_FAULTS::OUTPUT_PIKSI_TOGGLED;
    if (dev_name == &Devices::spike_and_hold()) group = GOMSPACE_FAULTS::OUTPUT_SPIKE_AND_HOLD_TOGGLED;
    if (dev_name == &Devices::quake()) group = GOMSPACE_FAULTS::OUTPUT_QUAKE_TOGGLED;
    if (dev_name == &Devices::adcs_system()) group = GOMSPACE_FAULTS::OUTPUT_ADCS_TOGGLED;
    
    int offset = 0;
    if (field == "TOGGLE") offset = 0;
    else if (field == "CURRENT") offset = 1;
    
    GOMSPACE_FAULTS fault = (GOMSPACE_FAULTS) (group + offset);
    set_error(fault, value);
}

static void gomspace_check() {
    debug_println("Checking Gomspace data...");
    
    debug_printf("Checking if Gomspace is functional...");
    bool is_gomspace_functional = State::read((State::Hardware::hat).at(&gomspace()).is_functional, 
                                    State::Hardware::hardware_state_lock);
    if (!is_gomspace_functional) {
        debug_println("Gomspace is not functional!");
        return;
    }
    else debug_println("Device is functional.");

    debug_println("Checking Gomspace inputs (currents and voltages).");
    unsigned short* vboosts = State::Gomspace::gomspace_data.vboost;
    unsigned short* curins = State::Gomspace::gomspace_data.curin;
    rwMtxRLock(&gomspace_state_lock);
        for (int i = 0; i < 3; i++) {
            if (vboosts[i] <= Constants::Gomspace::boost_voltage_limits.min
                || vboosts[i] >= Constants::Gomspace::boost_voltage_limits.max) {
                set_error((GOMSPACE_FAULTS) (GOMSPACE_FAULTS::BOOST_VOLTAGE_1 + i), true);
            }
            else
                set_error((GOMSPACE_FAULTS) (GOMSPACE_FAULTS::BOOST_VOLTAGE_1 + i), false);
        }
        for (int i = 0; i < 3; i++) {
            if (curins[i] <= Constants::Gomspace::individual_boost_converter_limits.min
                || curins[i] >= Constants::Gomspace::individual_boost_converter_limits.max) {
                set_error((GOMSPACE_FAULTS) (GOMSPACE_FAULTS::BOOST_CURRENT_1 + i), true);
            }
            else
                set_error((GOMSPACE_FAULTS) (GOMSPACE_FAULTS::BOOST_CURRENT_1 + i), false);
        }
        unsigned short cursun = State::Gomspace::gomspace_data.cursun;
        if (cursun <= Constants::Gomspace::total_boost_converter_limits.min
            || cursun >= Constants::Gomspace::total_boost_converter_limits.max) {
            set_error(GOMSPACE_FAULTS::BOOST_CURRENT_TOTAL, true);
        }
        else
            set_error(GOMSPACE_FAULTS::BOOST_CURRENT_TOTAL, false);
    rwMtxRLock(&gomspace_state_lock);

    debug_println("Checking Gomspace outputs (currents and voltages).");
    unsigned char* outputs = State::Gomspace::gomspace_data.output;
    unsigned short* currents = State::Gomspace::gomspace_data.curout;
    rwMtxRLock(&gomspace_state_lock);
        if(State::Hardware::hat.at(&Devices::quake()).powered_on != outputs[Gomspace::DEVICE_PINS::QUAKE])
            set_hardware_error(&Devices::quake(), "TOGGLE", true);
        else
            set_hardware_error(&Devices::quake(), "TOGGLE", false);
        if(State::Hardware::hat.at(&Devices::adcs_system()).powered_on != outputs[Gomspace::DEVICE_PINS::ADCS])
            set_hardware_error(&Devices::adcs_system(), "TOGGLE", true);
        else
            set_hardware_error(&Devices::adcs_system(), "TOGGLE", false);
        if(State::Hardware::hat.at(&Devices::spike_and_hold()).powered_on != outputs[Gomspace::DEVICE_PINS::SPIKE_AND_HOLD])
            set_hardware_error(&Devices::spike_and_hold(), "TOGGLE", true);
        else
            set_hardware_error(&Devices::spike_and_hold(), "TOGGLE", false);
        if(State::Hardware::hat.at(&Devices::piksi()).powered_on != outputs[Gomspace::DEVICE_PINS::PIKSI])
            set_hardware_error(&Devices::piksi(), "TOGGLE", true);
        else
            set_hardware_error(&Devices::piksi(), "TOGGLE", false);

        if(currents[Gomspace::DEVICE_PINS::QUAKE] >= Constants::Gomspace::quake_limits.min
            || currents[Gomspace::DEVICE_PINS::QUAKE] >= Constants::Gomspace::quake_limits.max)
            set_hardware_error(&Devices::quake(), "CURRENT", true);
        else
            set_hardware_error(&Devices::quake(), "CURRENT", false);
        if(currents[Gomspace::DEVICE_PINS::ADCS] >= Constants::Gomspace::adcs_system_limits.min
            || currents[Gomspace::DEVICE_PINS::ADCS] >= Constants::Gomspace::adcs_system_limits.max)
            set_hardware_error(&Devices::adcs_system(), "CURRENT", true);
        else
            set_hardware_error(&Devices::adcs_system(), "CURRENT", false);
        if(currents[Gomspace::DEVICE_PINS::SPIKE_AND_HOLD] >= Constants::Gomspace::spike_and_hold_limits.min
            || currents[Gomspace::DEVICE_PINS::SPIKE_AND_HOLD] >= Constants::Gomspace::spike_and_hold_limits.max)
            set_hardware_error(&Devices::spike_and_hold(), "CURRENT", true);
        else
            set_hardware_error(&Devices::spike_and_hold(), "CURRENT", false);
        if(currents[Gomspace::DEVICE_PINS::PIKSI] >= Constants::Gomspace::piksi_limits.min
            || currents[Gomspace::DEVICE_PINS::PIKSI] >= Constants::Gomspace::piksi_limits.max)
            set_hardware_error(&Devices::piksi(), "CURRENT", true);
        else
            set_hardware_error(&Devices::piksi(), "CURRENT", false);

        unsigned short cursys = State::Gomspace::gomspace_data.cursys;
        if(cursys >= Constants::Gomspace::battery_current_limits.min
           || cursys >= Constants::Gomspace::battery_current_limits.max) {
            set_error(GOMSPACE_FAULTS::BATTERY_CURRENT, false);
        }
        else
            set_error(GOMSPACE_FAULTS::BATTERY_CURRENT, false);
    rwMtxRLock(&gomspace_state_lock);

    debug_println("Checking Gomspace temperature.");
    short* temps = State::Gomspace::gomspace_data.temp;
    rwMtxRLock(&gomspace_state_lock);
        for(int i = 0; i < 4; i++) {
            if (temps[i] <= Constants::Gomspace::temperature_limits.min &&
                temps[i] >= Constants::Gomspace::temperature_limits.max) {
                set_error((GOMSPACE_FAULTS) (GOMSPACE_FAULTS::TEMPERATURE_1 + i), true);
            }
            else 
                set_error((GOMSPACE_FAULTS) (GOMSPACE_FAULTS::TEMPERATURE_1 + i), false);
        }
    rwMtxRUnlock(&gomspace_state_lock);
}

static THD_WORKING_AREA(gomspace_read_controller_workingArea, 4096);
static void gomspace_read_controller(void *arg) {
    chRegSetThreadName("GS READ");
    debug_println("Starting Gomspace reading and checking process.");

    systime_t time = chVTGetSystemTimeX(); // T0
    while (true) {
        time += MS2ST(RTOSTasks::LoopTimes::GOMSPACE);

        gomspace_read();
        gomspace_check();

        chThdSleepUntil(time);
    }
}

void RTOSTasks::gomspace_controller(void *arg) {
    chRegSetThreadName("GOMSPACE");
    debug_println("Gomspace controller process has started.");
    (void)chThdCreateStatic(gomspace_read_controller_workingArea, 
        sizeof(gomspace_read_controller_workingArea),
        RTOSTasks::gomspace_thread_priority, gomspace_read_controller, NULL);
    
    debug_println("Waiting for deployment timer to finish.");
    bool is_deployed = State::read(State::Master::is_deployed, State::Master::master_state_lock);
    if (!is_deployed) chThdEnqueueTimeoutS(&deployment_timer_waiting, S2ST(DEPLOYMENT_LENGTH));
    debug_println("Deployment timer has finished.");
    debug_println("Initializing main operation...");

    chThdExit((msg_t)0);
}