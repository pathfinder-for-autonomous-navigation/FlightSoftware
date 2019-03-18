#include "master_helpers.hpp"
#include "../../state/device_states.hpp"
#include "../../state/fault_state_holder.hpp"
#include "../gomspace/power_cyclers.hpp"
#include <rt/chdynamic.h>

struct power_cycler_args {
    bool adcs_system;
    bool spike_and_hold;
    bool piksi;
    bool quake;
    bool gomspace;
};
static THD_WORKING_AREA(power_cycler_wA, 256);
static THD_FUNCTION(power_cycler, args) {
    power_cycler_args* pcargs = (power_cycler_args*) args;

    if (pcargs->gomspace) {
        chMtxLock(&State::Hardware::gomspace_device_lock);
            Devices::gomspace().reboot();
        chMtxUnlock(&State::Hardware::gomspace_device_lock); // Note: this actually never gets called, 
                                                             // since Gomspace reboots
        chThdExit((msg_t) 0); // Note: this actually never gets called, since Gomspace reboots
    }

    if (pcargs->adcs_system) {
        Gomspace::cycler_arg_t cycler_args = {
            &State::Hardware::adcs_device_lock,
            &Devices::adcs_system(),
            Devices::Gomspace::DEVICE_PINS::ADCS
        };
        if (Gomspace::adcs_system_thread == NULL)
            Gomspace::adcs_system_thread = chThdCreateFromMemoryPool(&Gomspace::power_cycler_pool,
                "POWER CYCLE ADCS", 
                RTOSTasks::master_thread_priority,
                Gomspace::cycler_fn, (void*) &cycler_args);
    }
    if (pcargs->spike_and_hold) {
        Gomspace::cycler_arg_t cycler_args = {
            &State::Hardware::spike_and_hold_device_lock,
            &Devices::spike_and_hold(),
            Devices::Gomspace::DEVICE_PINS::SPIKE_AND_HOLD
        };
        if (Gomspace::spike_and_hold_thread == NULL)
            Gomspace::spike_and_hold_thread = chThdCreateFromMemoryPool(&Gomspace::power_cycler_pool,
                "POWER CYCLE SPIKE AND HOLD", 
                RTOSTasks::master_thread_priority,
                Gomspace::cycler_fn, (void*) &cycler_args);
    }
    if (pcargs->piksi) {
        Gomspace::cycler_arg_t cycler_args = {
            &State::Hardware::piksi_device_lock,
            &Devices::piksi(),
            Devices::Gomspace::DEVICE_PINS::PIKSI
        };
        if (Gomspace::piksi_thread == NULL)
            Gomspace::piksi_thread = chThdCreateFromMemoryPool(&Gomspace::power_cycler_pool,
                "POWER CYCLE PIKSI", 
                RTOSTasks::master_thread_priority,
                Gomspace::cycler_fn, (void*) &cycler_args);
    }
    if (pcargs->quake) {
        Gomspace::cycler_arg_t cycler_args = {
            &State::Hardware::quake_device_lock,
            &Devices::quake(),
            Devices::Gomspace::DEVICE_PINS::QUAKE
        };
        if (Gomspace::quake_thread == NULL)
            Gomspace::quake_thread = chThdCreateFromMemoryPool(&Gomspace::power_cycler_pool,
                "POWER CYCLE QUAKE",
                RTOSTasks::master_thread_priority,
                Gomspace::cycler_fn, (void*) &cycler_args);
    }

    if (Gomspace::adcs_system_thread != NULL) {
        chThdWait(Gomspace::adcs_system_thread);
        Gomspace::adcs_system_thread = NULL;
    }
    if (Gomspace::spike_and_hold_thread != NULL) {
        chThdWait(Gomspace::spike_and_hold_thread);
        Gomspace::spike_and_hold_thread = NULL;
    }
    if (Gomspace::piksi_thread != NULL) {
        chThdWait(Gomspace::piksi_thread);
        Gomspace::piksi_thread = NULL;
    }
    if (Gomspace::quake_thread != NULL) {
        chThdWait(Gomspace::quake_thread);
        Gomspace::quake_thread = NULL;
    }
    chThdExit((msg_t) 0);
}

static THD_WORKING_AREA(docking_motor_toggler_wA, 256);
static THD_FUNCTION(docking_motor_toggler, args) {
    bool docking_motor_docked = *((bool*) args);
    if (docking_motor_docked) {
        if (State::Hardware::check_is_functional(&Devices::docking_motor()))
            Devices::docking_motor().dock();
    }
    else {
        if (State::Hardware::check_is_functional(&Devices::docking_motor()))
            Devices::docking_motor().undock();
    }
    chThdExit((msg_t) 0);
}

void Master::apply_uplink_data() {
    Comms::Uplink &uplink = State::Quake::most_recent_uplink;

    rwMtxRLock(&State::Quake::uplink_lock);
    std::array<double, 3> p = uplink.other_satellite_position;
    std::array<double, 3> v = uplink.other_satellite_velocity;
    gps_time_t t = uplink.other_satellite_timestamp;
    rwMtxRUnlock(&State::Quake::uplink_lock);
    
    rwMtxWLock(&State::Piksi::piksi_state_lock);
        bool rtk_lock = State::Piksi::is_fixed_rtk || State::Piksi::is_float_rtk;
        if (!rtk_lock) {
            State::Piksi::recorded_gps_position_other = p;
            State::Piksi::recorded_gps_position_other = v;
            State::Piksi::recorded_gps_position_other_time = t;
        }
    rwMtxWUnlock(&State::Piksi::piksi_state_lock);

    apply_uplink_adcs_hat(uplink);
    apply_uplink_fc_hat(uplink);
    apply_uplink_constants(uplink);
}

void Master::apply_uplink_commands() {
    Comms::Uplink uplink = State::read(State::Quake::most_recent_uplink, State::Quake::uplink_lock);
    
    // Master state handling
    State::Master::MasterState ms = (State::Master::MasterState) uplink.master_state;
    State::Master::PANState ps = (State::Master::PANState) uplink.pan_state;
    bool was_safehold = State::read(State::Master::master_state, State::Master::master_state_lock) == State::Master::MasterState::SAFE_HOLD;
    bool is_safehold = ms == State::Master::MasterState::SAFE_HOLD;
    bool is_standby = ps == State::Master::PANState::STANDBY;

    // Disable safe hold timer if being commanded out of sate hold
    if (was_safehold && !is_safehold) {
        chThdTerminate(safe_hold_timer_thread);
        safe_hold_timer_thread = NULL;
        State::write(State::Master::autoexited_safe_hold, false, State::Master::master_state_lock);
    }

    // Write "ignore error" bits
    rwMtxWLock(&FaultState::Propulsion::propulsion_faults_state_lock);
        FaultState::Propulsion::cannot_pressurize_outer_tank_ignored = uplink.cannot_pressurize_outer_tank_ignored;
        FaultState::Propulsion::destabilization_event.is_not_set = uplink.ignore_destabilized;
        FaultState::Propulsion::overpressure_event.is_not_set = uplink.ignore_overpressure;
    rwMtxWUnlock(&FaultState::Propulsion::propulsion_faults_state_lock);
    State::write(FaultState::Gomspace::vbatt_ignored, 
                    uplink.vbatt_ignored, 
                    FaultState::Gomspace::gomspace_faults_state_lock);
    rwMtxWLock(&FaultState::ADCS::adcs_faults_state_lock);
        FaultState::ADCS::all_magnetometers_faulty_ignore = uplink.all_magnetometers_faulty_ignore;
        FaultState::ADCS::all_ssa_faulty_ignore = uplink.all_ssa_faulty_ignore;
        FaultState::ADCS::motor_x_faulty_ignore = uplink.motor_x_faulty_ignore;
        FaultState::ADCS::motor_y_faulty_ignore = uplink.motor_y_faulty_ignore;
        FaultState::ADCS::motor_z_faulty_ignore = uplink.motor_z_faulty_ignore;
    rwMtxWLock(&FaultState::ADCS::adcs_faults_state_lock);

    // Command state
    State::write(State::Master::master_state, ms, State::Master::master_state_lock);
    State::write(State::Master::pan_state, ps, State::Master::master_state_lock);

    // ADCS
    State::ADCS::ADCSState as = (State::ADCS::ADCSState) uplink.adcs_state;
    if (is_standby) {
        State::write(State::ADCS::adcs_state, as, State::ADCS::adcs_state_lock);
        std::array<float, 4> cmd_attitude = uplink.adcs_attitude;
        State::ADCS::PointingFrame frame = (State::ADCS::PointingFrame) uplink.adcs_frame;                                            
        State::write(State::ADCS::cmd_attitude, cmd_attitude, State::ADCS::adcs_state_lock);
        State::write(State::ADCS::cmd_attitude_frame, frame, State::ADCS::adcs_state_lock);
    }
    State::ADCS::ADCSGainState gain_state = (State::ADCS::ADCSGainState) uplink.adcs_gain_state;
    if (gain_state == State::ADCS::ADCSGainState::NORMAL) {
        // TODO set gains
    }
    else if (gain_state == State::ADCS::ADCSGainState::PAIRED) {
        // TODO set gains   
    }

    // Propulsion
    bool command_propulsion = uplink.command_propulsion;
    if (command_propulsion && is_standby) {
        std::array<float, 3> impulse_vector = uplink.firing_vector;
        gps_time_t impulse_time = uplink.firing_time;
        State::write(State::Propulsion::firing_data.impulse_vector, impulse_vector, State::Propulsion::propulsion_state_lock);
        State::write(State::Propulsion::firing_data.time, impulse_time, State::Propulsion::propulsion_state_lock);
    }

    // Resets
    if (uplink.reset_dcdc) {
        chMtxLock(&State::Hardware::dcdc_device_lock);
            Devices::dcdc().reset();
        chMtxUnlock(&State::Hardware::dcdc_device_lock);
    }
    if (uplink.reset_spike_and_hold) {
        chMtxLock(&State::Hardware::spike_and_hold_device_lock);
            Devices::spike_and_hold().reset();
        chMtxUnlock(&State::Hardware::spike_and_hold_device_lock);
    }
    if (uplink.reset_piksi) {
        chMtxLock(&State::Hardware::piksi_device_lock);
            Devices::piksi().reset();
        chMtxUnlock(&State::Hardware::piksi_device_lock);
    }
    if (uplink.reset_quake) {
        chMtxLock(&State::Hardware::quake_device_lock);
            Devices::quake().reset();
        chMtxUnlock(&State::Hardware::quake_device_lock);
    }

    // Power cycles
    power_cycler_args args;
    args.adcs_system = uplink.power_cycle_adcs_system;
    args.quake = uplink.power_cycle_quake;
    args.piksi = uplink.power_cycle_piksi;
    args.gomspace = uplink.power_cycle_gomspace;
    args.spike_and_hold = uplink.power_cycle_spike_and_hold;
    chThdCreateStatic(power_cycler_wA, sizeof(power_cycler_wA), 
        RTOSTasks::master_thread_priority, power_cycler, (void*)&args);
    
    // Manage docking motor
    chThdCreateStatic(docking_motor_toggler_wA, sizeof(docking_motor_toggler_wA), 
        RTOSTasks::master_thread_priority, docking_motor_toggler, (void*)&(uplink.docking_motor_mode));
}