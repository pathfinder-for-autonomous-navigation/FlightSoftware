#include "master_helpers.hpp"
#include "../../state/device_states.hpp"

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
            Devices::gomspace.reboot();
        chMtxUnlock(&State::Hardware::gomspace_device_lock); // Note: this actually never gets called, 
                                                             // since Gomspace reboots
        chThdExit((msg_t) 0); // Note: this actually never gets called, since Gomspace reboots
    }
    if (pcargs->adcs_system) {
        chMtxLock(&State::Hardware::gomspace_device_lock);
            Devices::gomspace.set_single_output(Devices::Gomspace::DEVICE_PINS::ADCS,0);
            chThdSleepSeconds(30);
            Devices::gomspace.set_single_output(Devices::Gomspace::DEVICE_PINS::ADCS,1);
        chMtxUnlock(&State::Hardware::gomspace_device_lock);
    }
    if (pcargs->spike_and_hold) {
        chMtxLock(&State::Hardware::gomspace_device_lock);
            Devices::gomspace.set_single_output(Devices::Gomspace::DEVICE_PINS::SPIKE_AND_HOLD,0);
            chThdSleepSeconds(30);
            Devices::gomspace.set_single_output(Devices::Gomspace::DEVICE_PINS::SPIKE_AND_HOLD,1);
        chMtxUnlock(&State::Hardware::gomspace_device_lock);
    }
    if (pcargs->piksi) {
        chMtxLock(&State::Hardware::gomspace_device_lock);
            Devices::gomspace.set_single_output(Devices::Gomspace::DEVICE_PINS::PIKSI,0);
            chThdSleepSeconds(30);
            Devices::gomspace.set_single_output(Devices::Gomspace::DEVICE_PINS::PIKSI,1);
        chMtxUnlock(&State::Hardware::gomspace_device_lock);
    }
    if (pcargs->quake) {
        chMtxLock(&State::Hardware::gomspace_device_lock);
            Devices::gomspace.set_single_output(Devices::Gomspace::DEVICE_PINS::QUAKE,0);
            chThdSleepSeconds(30);
            Devices::gomspace.set_single_output(Devices::Gomspace::DEVICE_PINS::QUAKE,1);
        chMtxUnlock(&State::Hardware::gomspace_device_lock);
    }
}

static THD_WORKING_AREA(docking_motor_toggler_wA, 256);
static THD_FUNCTION(docking_motor_toggler, args) {
    bool docking_motor_docked = *((bool*) args);
    if (docking_motor_docked) {
        // TODO
    }
    else {
        // TODO
    }
    chThdExit((msg_t) 0);
}

void Master::apply_uplink_data() {
    Comms::Uplink uplink = State::read(State::Quake::most_recent_uplink, State::Quake::uplink_lock);

    std::array<double, 3> p = uplink.other_satellite_position;
    std::array<double, 3> v = uplink.other_satellite_velocity;
    gps_time_t t = uplink.other_satellite_timestamp;
    
    bool rtk_lock = false; // TODO
    if (!rtk_lock) {
        State::write(State::Piksi::recorded_gps_position_other, p, State::Piksi::piksi_state_lock);
        State::write(State::Piksi::recorded_gps_position_other, v, State::Piksi::piksi_state_lock);
        State::write(State::Piksi::recorded_gps_position_other_time, t, State::Piksi::piksi_state_lock);
    }

    apply_uplink_adcs_hat(uplink);
    apply_uplink_fc_hat(uplink);
    apply_uplink_constants(uplink);
}

void Master::apply_uplink_commands() {
    Comms::Uplink uplink = State::read(State::Quake::most_recent_uplink, State::Quake::uplink_lock);
    
    // Master state handling
    State::Master::MasterState ms = (State::Master::MasterState) uplink.master_state;
    State::Master::PANState ps = (State::Master::PANState) uplink.pan_state;
    bool is_standby = ps == State::Master::PANState::STANDBY;
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
            Devices::dcdc.reset();
        chMtxUnlock(&State::Hardware::dcdc_device_lock);
    }
    if (uplink.reset_spike_and_hold) {
        chMtxLock(&State::Hardware::spike_and_hold_device_lock);
            Devices::spike_and_hold.reset();
        chMtxUnlock(&State::Hardware::spike_and_hold_device_lock);
    }
    if (uplink.reset_piksi) {
        chMtxLock(&State::Hardware::piksi_device_lock);
            Devices::piksi.reset();
        chMtxUnlock(&State::Hardware::piksi_device_lock);
    }
    if (uplink.reset_quake) {
        chMtxLock(&State::Hardware::quake_device_lock);
            Devices::quake.reset();
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