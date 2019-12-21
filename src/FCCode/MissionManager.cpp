#include "MissionManager.hpp"

MissionManager::MissionManager(StateFieldRegistry& registry, unsigned int offset) :
    TimedControlTask<void>(registry, "mission_ct", offset),
    mission_mode_sr(10),
    mission_mode_f("pan.mode", mission_mode_sr),
    is_deployed_sr(),
    is_deployed_f("pan.deployed", is_deployed_sr),
    sat_designation_sr(2),
    sat_designation_f("pan.sat_designation", sat_designation_sr)
{
    add_writable_field(mission_mode_f);
    add_readable_field(is_deployed_f);
    add_writable_field(sat_designation_f);

    adcs_mode_fp = find_writable_field<unsigned char>("adcs.mode", __FILE__, __LINE__);
    adcs_cmd_attitude_fp = find_writable_field<f_quat_t>("adcs.cmd_attitude", __FILE__, __LINE__);
    adcs_ang_rate_fp = find_readable_field<float>("adcs.ang_rate", __FILE__, __LINE__);
    adcs_min_stable_ang_rate_fp = find_writable_field<float>("adcs.min_stable_ang_rate", __FILE__, __LINE__);
 
    radio_mode_fp = find_readable_field<unsigned char>("radio.mode", __FILE__, __LINE__);

    mission_mode_f.set(static_cast<unsigned int>(mission_mode_t::startup));
    is_deployed_f.set(false);
    sat_designation_f.set(0);
}

void MissionManager::execute() {
    mission_mode_t mode = static_cast<mission_mode_t>(mission_mode_f.get());
    switch(mode) {
        case mission_mode_t::startup:
            dispatch_startup();
            break;
        case mission_mode_t::detumble:
            dispatch_detumble();
            break;
        case mission_mode_t::initialization_hold:
            dispatch_initialization_hold();
            break;
        case mission_mode_t::follower:
            dispatch_follower();
            break;
        case mission_mode_t::follower_close_approach:
            dispatch_follower_close_approach();
            break;
        case mission_mode_t::standby:
            dispatch_standby();
            break;
        case mission_mode_t::leader_close_approach:
            dispatch_leader_close_approach();
            break;
        case mission_mode_t::docking:
            dispatch_docking();
            break;
        case mission_mode_t::docked:
            dispatch_docked();
            break;
        case mission_mode_t::paired:
            dispatch_paired();
            break;
        case mission_mode_t::spacejunk:
            dispatch_spacejunk();
            break;
        case mission_mode_t::safehold:
            dispatch_safehold();
            break;
        default:
            printf(debug_severity::error, "Master state not defined: %d\n", static_cast<unsigned int>(mode));
            mission_mode_f.set(static_cast<unsigned int>(mission_mode_t::safehold));
            break;
    }
}

void MissionManager::dispatch_startup() {
    // For now, do absolutely nothing. Wait for the startup controller to
    // bring us out of this mode.
}

void MissionManager::dispatch_detumble() {
    mission_mode_f.set(static_cast<unsigned int>(mission_mode_t::detumble));
    adcs_mode_fp->set(static_cast<unsigned int>(adcs_mode_t::detumble));
    if (adcs_ang_rate_fp->get() <= adcs_min_stable_ang_rate_fp->get())
    {
        adcs_cmd_attitude_fp->set({2,2,2,2}); // TODO fix to a good value
        adcs_mode_fp->set(static_cast<unsigned int>(adcs_mode_t::point_standby));
        mission_mode_f.set(static_cast<unsigned int>(mission_mode_t::standby));
    }
}

void MissionManager::dispatch_initialization_hold() {
    // TODO
}

void MissionManager::dispatch_follower() {
    
    // TODO
}

void MissionManager::dispatch_follower_close_approach() {
    // TODO
}

void MissionManager::dispatch_standby() {
    // TODO
}

void MissionManager::dispatch_leader_close_approach() {
    // TODO
}

void MissionManager::dispatch_docking() {
    // TODO
}

void MissionManager::dispatch_docked() {
    // TODO
}

void MissionManager::dispatch_paired() {
    // TODO
}

void MissionManager::dispatch_spacejunk() {
    // TODO
}

void MissionManager::dispatch_safehold() {
    // TODO
}
