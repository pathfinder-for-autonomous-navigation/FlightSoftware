/** @file controllers/gomspace.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Contains implementation for the gomspace state controller.
 */

#include "controllers.hpp"
#include "constants.hpp"
#include "../state/state_holder.hpp"
#include "../state/fault_state_holder.hpp"
#include "../deployment_timer.hpp"

using Devices::Gomspace;
using Devices::gomspace;
using State::Gomspace::gomspace_data;
using FaultState::Gomspace::GOMSPACE_FAULTS;

namespace RTOSTasks {
    THD_WORKING_AREA(gomspace_controller_workingArea, 2048);
}

namespace Constants {
namespace Gomspace {
    static limit_t quake_limits = {0,0};
    static limit_t adcs_system_limits = {0,0};
    static limit_t spike_and_hold_limits = {0,0};
    static limit_t piksi_limits = {0,0};
    static limit_t individual_boost_converter_limits = {0,0};
    static limit_t total_boost_converter_limits = {0,0};
    static limit_t battery_current_limits = {0,0};
    std::map<std::string, limit_t&> current_limits {
        {Devices::quake.name(), quake_limits},
        {Devices::adcs_system.name(), adcs_system_limits},
        {Devices::spike_and_hold.name(), spike_and_hold_limits},
        {Devices::piksi.name(), piksi_limits},
        {"Individual Boost Converter", individual_boost_converter_limits},
        {"Total Boost Converter", total_boost_converter_limits},
        {"Battery Current", battery_current_limits}
    };
}
}

static void gomspace_read() {
    debug_printf("Reading Gomspace data...");
    unsigned char t = 0; // # of tries at reading housekeeping data
    while (t < 5) {
        if (gomspace.get_hk()) break;
        t++;
    }
    if (t == 5) {
        debug_println("unable to read Gomspace data.");
        rwMtxWLock(&State::Hardware::hat_lock);
            (State::Hardware::hat).at(Devices::gomspace.name()).is_functional = false;
        rwMtxWUnlock(&State::Hardware::hat_lock);
    }
    else debug_printf("battery voltage (mV): %d\n", gomspace_data.vbatt);
}

static void set_error(GOMSPACE_FAULTS fault, bool value) {
    rwMtxWLock(&FaultState::Gomspace::gomspace_fault_state_lock);
        FaultState::Gomspace::fault_bits.set(fault, value);
    rwMtxWUnlock(&FaultState::Gomspace::gomspace_fault_state_lock);
}

static void set_hardware_error(const std::string& dev_name, const std::string field, bool value) {
    GOMSPACE_FAULTS group;
    if (dev_name == Devices::piksi.name()) group = GOMSPACE_FAULTS::OUTPUT_PIKSI_TOGGLED;
    if (dev_name == Devices::spike_and_hold.name()) group = GOMSPACE_FAULTS::OUTPUT_SPIKE_AND_HOLD_TOGGLED;
    if (dev_name == Devices::quake.name()) group = GOMSPACE_FAULTS::OUTPUT_QUAKE_TOGGLED;
    if (dev_name == Devices::adcs_system.name()) group = GOMSPACE_FAULTS::OUTPUT_ADCS_TOGGLED;
    
    int offset = 0;
    if (field == "TOGGLE") offset = 0;
    else if (field == "CURRENT") offset = 1;
    
    GOMSPACE_FAULTS fault = (GOMSPACE_FAULTS) (group + offset);
    set_error(fault, value);
}

static void gomspace_check() {
    debug_println("Checking Gomspace data...");
    
    debug_printf("Checking if Gomspace is functional...");
    rwMtxRLock(&State::Hardware::hat_lock);
    bool is_gomspace_functional = (State::Hardware::hat).at(Devices::gomspace.name()).is_functional;
    rwMtxRUnlock(&State::Hardware::hat_lock);
    if (!is_gomspace_functional) {
        debug_println("Gomspace is not functional!");
        return;
    }
    else debug_println("Device is functional.");

    debug_printf("Checking Gomspace battery voltage...");
    rwMtxRLock(&State::Gomspace::gomspace_state_lock);
    unsigned short int vbatt = gomspace_data.vbatt;
    rwMtxRUnlock(&State::Gomspace::gomspace_state_lock);
    if (vbatt < Constants::Gomspace::SAFE_VOLTAGE) {
        rwMtxWLock(&FaultState::Gomspace::gomspace_fault_state_lock);
            FaultState::Gomspace::is_safe_hold_voltage = true;
        rwMtxWUnlock(&FaultState::Gomspace::gomspace_fault_state_lock);
    }

    debug_println("Checking Gomspace inputs (currents and voltages).");
    unsigned short* vboosts = State::Gomspace::gomspace_data.vboost;
    unsigned short* curins = State::Gomspace::gomspace_data.curin;
    rwMtxRLock(&State::Gomspace::gomspace_state_lock);
        for (int i = 0; i < 3; i++) {
            if (vboosts[i] <= Constants::Gomspace::boost_voltage_limits.min
                || vboosts[i] >= Constants::Gomspace::boost_voltage_limits.max) {
                set_error((GOMSPACE_FAULTS) (GOMSPACE_FAULTS::BOOST_VOLTAGE_1 + i), true);
            }
            else {
                set_error((GOMSPACE_FAULTS) (GOMSPACE_FAULTS::BOOST_VOLTAGE_1 + i), false);
            }
        }
        for (int i = 0; i < 3; i++) {
            if (curins[i] <= Constants::Gomspace::current_limits.at("Individual Boost Converter").min
                || curins[i] >= Constants::Gomspace::current_limits.at("Individual Boost Converter").max) {
                set_error((GOMSPACE_FAULTS) (GOMSPACE_FAULTS::BOOST_CURRENT_1 + i), true);
            }
            else {
                set_error((GOMSPACE_FAULTS) (GOMSPACE_FAULTS::BOOST_CURRENT_1 + i), false);
            }
        }
        unsigned short cursun = State::Gomspace::gomspace_data.cursun;
        if (cursun <= Constants::Gomspace::current_limits.at("Total Boost Converter").min
            || cursun >= Constants::Gomspace::current_limits.at("Total Boost Converter").max) {
            set_error(GOMSPACE_FAULTS::BOOST_CURRENT_TOTAL, true);
        }
        else {
            set_error(GOMSPACE_FAULTS::BOOST_CURRENT_TOTAL, false);
        }
    rwMtxRLock(&State::Gomspace::gomspace_state_lock);


    debug_println("Checking Gomspace outputs (currents and voltages).");
    unsigned char* outputs = State::Gomspace::gomspace_data.output;
    unsigned short* currents = State::Gomspace::gomspace_data.curout;
    rwMtxRLock(&State::Gomspace::gomspace_state_lock);
        for(auto dev : State::Hardware::power_outputs) {
            if(State::Hardware::hat.at(dev.first).powered_on != outputs[dev.second]) {
                set_hardware_error(dev.first, "TOGGLE", true);
            }
            else {
                set_hardware_error(dev.first, "TOGGLE", false);
            }
        }
        for(auto dev : State::Hardware::power_outputs) {
            if(currents[dev.second] >= Constants::Gomspace::current_limits.at(dev.first).min
                || currents[dev.second] >= Constants::Gomspace::current_limits.at(dev.first).max) {
                set_hardware_error(dev.first, "CURRENT", true); 
            }
            else {
                set_hardware_error(dev.first, "CURRENT", false);
            }
        }
        unsigned short cursys = State::Gomspace::gomspace_data.cursys;
        if(cursys >= Constants::Gomspace::current_limits.at("Battery Current").min
           || cursys >= Constants::Gomspace::current_limits.at("Battery Current").max) {
            set_error(GOMSPACE_FAULTS::BATTERY_CURRENT, false);
        }
        else {
            set_error(GOMSPACE_FAULTS::BATTERY_CURRENT, false);
        }
    rwMtxRLock(&State::Gomspace::gomspace_state_lock);

    debug_println("Checking Gomspace temperature.");
    short* temps = State::Gomspace::gomspace_data.temp;
    rwMtxRLock(&State::Gomspace::gomspace_state_lock);
        for(int i = 0; i < 4; i++) {
            if (temps[i] <= Constants::Gomspace::temperature_limits.min &&
                temps[i] >= Constants::Gomspace::temperature_limits.max) {
                set_error((GOMSPACE_FAULTS) (GOMSPACE_FAULTS::TEMPERATURE_1 + i), true);
            }
            else {
                set_error((GOMSPACE_FAULTS) (GOMSPACE_FAULTS::TEMPERATURE_1 + i), false);
            }
        }
    rwMtxRUnlock(&State::Gomspace::gomspace_state_lock);
}

static THD_WORKING_AREA(gomspace_read_controller_workingArea, 4096);
static void gomspace_read_controller(void *arg) {
    chRegSetThreadName("GS READ");
    debug_println("Starting Gomspace reading and checking process.");

    systime_t time = chVTGetSystemTimeX(); // T0
    while (true) {
        time += MS2ST(RTOSTasks::LoopTimes::GOMSPACE);

        if (State::Hardware::can_get_data(Devices::gomspace)) {
            gomspace_read();
            gomspace_check();
        }

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
    rwMtxRLock(&State::Master::master_state_lock);
        bool is_deployed = State::Master::is_deployed;
    rwMtxRUnlock(&State::Master::master_state_lock);
    if (!is_deployed) chThdEnqueueTimeoutS(&deployment_timer_waiting, S2ST(DEPLOYMENT_LENGTH));
    debug_println("Deployment timer has finished.");
    debug_println("Initializing main operation...");

    chThdExit((msg_t)0);
}