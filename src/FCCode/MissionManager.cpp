#include "MissionManager.hpp"

MissionManager::MissionManager(StateFieldRegistry& registry) : ControlTask<void>(registry),
    mission_mode_sr(0, 10, 4),
    mission_mode_f("pan.mode", mission_mode_sr),
    is_deployed_sr(),
    is_deployed_f("pan.deployed", is_deployed_sr)
{
    add_writable(mission_mode_f);
    add_readable(is_deployed_f);

    find_readable_field("pan.cycle_no", &control_cycle_count_fp, __FILE__, __LINE__);
    find_writable_field("adcs.mode", &adcs_mode_fp, __FILE__, __LINE__);
    find_writable_field("adcs.cmd_attitude", &adcs_cmd_attitude_fp, __FILE__, __LINE__);

    mission_mode_f.set(static_cast<unsigned int>(mission_mode_t::detumble));
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
    adcs_mode_fp->set(static_cast<unsigned int>(mission_mode_t::detumble));
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
