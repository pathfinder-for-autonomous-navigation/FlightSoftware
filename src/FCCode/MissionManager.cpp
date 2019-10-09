#include "MissionManager.hpp"

MissionManager::MissionManager(StateFieldRegistry& registry) : 
    ControlTask<void>("pan.manager", registry),
    mission_mode_serializer(0, 10, 4),
    mission_mode_f("pan.mode", mission_mode_serializer),
    is_deployed_serializer(),
    is_deployed_f("pan.deployed", is_deployed_serializer)
{
    println(debug_severity::info, "Made it");
    std::shared_ptr<WritableStateField<unsigned int>> mission_mode_f_ptr(
        std::shared_ptr<WritableStateField<unsigned int>>{}, &mission_mode_f);
    registry.add_writable(mission_mode_f_ptr);
    
    std::shared_ptr<ReadableStateField<bool>> is_deployed_f_ptr(
        std::shared_ptr<ReadableStateField<bool>>{}, &is_deployed_f);
    registry.add_readable(is_deployed_f_ptr);

    adcs_mode_fp = std::dynamic_pointer_cast<WritableStateField<unsigned int>>(registry.find_writable_field("adcs.mode"));
    if (!adcs_mode_fp) { print_registry_404_error("adcs.mode"); }

    adcs_cmd_attitude_fp = std::dynamic_pointer_cast<WritableStateField<std::array<float, 3>>>(registry.find_writable_field("adcs.cmd_attitude"));
    if (!adcs_cmd_attitude_fp) { print_registry_404_error("adcs.cmd_attitude"); }

    mission_mode_f.set(static_cast<unsigned int>(mission_mode_t::detumble));
}

void MissionManager::execute() {
    mission_mode_t mode = static_cast<mission_mode_t>(mission_mode_f.get());
    switch(mode) {
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
    }
}

void MissionManager::dispatch_detumble() {
    adcs_mode_fp->set(3);
    // TODO
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
