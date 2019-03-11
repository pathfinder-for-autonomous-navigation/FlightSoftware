#include "master_helpers.hpp"

void Master::apply_uplink_data() {
    std::array<double, 3> p = State::read(State::Quake::most_recent_uplink.other_satellite_position, 
                                           State::Quake::uplink_lock);
    std::array<double, 3> v = State::read(State::Quake::most_recent_uplink.other_satellite_velocity, 
                                           State::Quake::uplink_lock);
    gps_time_t t = State::read(State::Quake::most_recent_uplink.other_satellite_timestamp, 
                                 State::Quake::uplink_lock);
    
    bool rtk_lock = false; // TODO
    if (!rtk_lock) {
        State::write(State::Piksi::recorded_gps_position_other, p, State::Piksi::piksi_state_lock);
        State::write(State::Piksi::recorded_gps_position_other, v, State::Piksi::piksi_state_lock);
        State::write(State::Piksi::recorded_gps_position_other_time, t, State::Piksi::piksi_state_lock);
    }

    // TODO Manage constants
    // TODO Manage HAVT
}

void Master::apply_uplink_commands() {
    // Master state handling
    State::Master::MasterState ms = (State::Master::MasterState) State::read(State::Quake::most_recent_uplink.master_state, 
                                                                            State::Quake::uplink_lock);
    State::Master::PANState ps = (State::Master::PANState) State::read(State::Quake::most_recent_uplink.pan_state, 
                                                                       State::Quake::uplink_lock);
    bool is_standby = ps == State::Master::PANState::STANDBY;
    State::write(State::Master::master_state, ms, State::Master::master_state_lock);
    State::write(State::Master::pan_state, ps, State::Master::master_state_lock);
    
    // ADCS
    State::ADCS::ADCSState as = (State::ADCS::ADCSState) State::read(State::Quake::most_recent_uplink.adcs_state,
                                                                     State::Quake::uplink_lock);
    if (is_standby) {
        State::write(State::ADCS::adcs_state, as, State::ADCS::adcs_state_lock);
        std::array<float, 4> cmd_attitude = State::read(State::Quake::most_recent_uplink.adcs_attitude, 
                                                                            State::Quake::uplink_lock);
        State::ADCS::PointingFrame frame = (State::ADCS::PointingFrame) State::read(State::Quake::most_recent_uplink.adcs_frame, 
                                                                                    State::Quake::uplink_lock);                                            
        State::write(State::ADCS::cmd_attitude, cmd_attitude, State::ADCS::adcs_state_lock);
        State::write(State::ADCS::cmd_attitude_frame, frame, State::ADCS::adcs_state_lock);
    }

    // Propulsion
    bool command_propulsion = State::read(State::Quake::most_recent_uplink.command_propulsion, State::Quake::uplink_lock);
    if (command_propulsion && is_standby) {
        std::array<float, 3> impulse_vector = State::read(State::Quake::most_recent_uplink.firing_vector, 
                                                          State::Quake::uplink_lock);
        gps_time_t impulse_time = State::read(State::Quake::most_recent_uplink.firing_time, 
                                              State::Quake::uplink_lock);
        State::write(State::Propulsion::firing_data.impulse_vector, impulse_vector, State::Propulsion::propulsion_state_lock);
        State::write(State::Propulsion::firing_data.time, impulse_time, State::Propulsion::propulsion_state_lock);
    }

    // TODO Manage resets/power cyclings
    // TODO Manage docking motor
}