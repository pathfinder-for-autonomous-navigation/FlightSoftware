#include "MissionManager.hpp"
#include <lin.hpp>
#include "propulsion_mode_t.enum"
#include "mission_state_t.enum"
#include "adcs_state_t.enum"
#include "radio_mode_t.enum"
#include "satellite_designation_t.enum"

#define set_mission_state(state) mission_state_f.set(static_cast<unsigned int>(mission_state_t::state));
#define set_adcs_state(state)    adcs_state_f.set(static_cast<unsigned int>(adcs_state_t::state));
#define set_prop_mode(mode)      prop_mode_fp->set(static_cast<unsigned int>(propulsion_mode_t::mode));
#define set_radio_mode(mode)     radio_mode_fp->set(static_cast<unsigned int>(radio_mode_t::mode));
#define set_satellite_designation(designation) sat_designation_f.set(static_cast<unsigned int>(satellite_designation_t::designation));

MissionManager::MissionManager(StateFieldRegistry& registry, unsigned int offset) :
    TimedControlTask<void>(registry, "mission_ct", offset),
    adcs_state_f("adcs.state", Serializer<unsigned char>(8)),
    docking_config_cmd_f("docksys.config_cmd", Serializer<bool>()),
    mission_state_f("pan.state", Serializer<unsigned char>(13)),
    is_deployed_f("pan.deployed", Serializer<bool>()),
    deployment_wait_elapsed_f("pan.deployment.elapsed", Serializer<unsigned int>(0, 15000, 32)),
    sat_designation_f("pan.sat_designation", Serializer<unsigned char>(2))
{
    add_writable_field(adcs_state_f);
    add_writable_field(docking_config_cmd_f);
    add_writable_field(mission_state_f);
    add_readable_field(is_deployed_f);
    add_writable_field(sat_designation_f);

    adcs_ang_vel_fp = find_readable_field<f_vector_t>("attitude_estimator.w_body", __FILE__, __LINE__);
    adcs_min_stable_ang_rate_fp = find_writable_field<float>("adcs.min_stable_ang_rate", __FILE__, __LINE__);

    radio_mode_fp = find_internal_field<unsigned char>("radio.mode", __FILE__, __LINE__);
    last_checkin_cycle_fp = find_internal_field<unsigned int>("radio.last_comms_ccno", __FILE__, __LINE__);

    prop_mode_fp = find_readable_field<unsigned char>("prop.mode", __FILE__, __LINE__);

    piksi_mode_fp = find_readable_field<unsigned char>("piksi.state", __FILE__, __LINE__);
    propagated_baseline_pos_fp = find_readable_field<d_vector_t>("orbit.baseline_pos", __FILE__, __LINE__);

    docked_fp = find_readable_field<bool>("docksys.docked", __FILE__, __LINE__);

    // Initialize a bunch of variables
    set_mission_state(startup);
    set_adcs_state(startup);
    set_prop_mode(disabled);
    set_radio_mode(disabled);
    is_deployed_f.set(false);
    deployment_wait_elapsed_f.set(0);
    sat_designation_f.set(0);
}

bool MissionManager::check_hardware_faults() {
    return false;
    // TODO
}

void MissionManager::execute() {
    mission_state_t mode = static_cast<mission_state_t>(mission_state_f.get());

    if (mode != mission_state_t::startup) {
        check_hardware_faults();
    }

    switch(mode) {
        case mission_state_t::startup:
            dispatch_startup();
            break;
        case mission_state_t::detumble:
            dispatch_detumble();
            break;
        case mission_state_t::initialization_hold:
            dispatch_initialization_hold();
            break;
        case mission_state_t::follower:
            dispatch_follower();
            break;
        case mission_state_t::follower_close_approach:
            dispatch_follower_close_approach();
            break;
        case mission_state_t::standby:
            dispatch_standby();
            break;
        case mission_state_t::leader_close_approach:
            dispatch_leader_close_approach();
            break;
        case mission_state_t::docking:
            dispatch_docking();
            break;
        case mission_state_t::docked:
            dispatch_docked();
            break;
        case mission_state_t::paired:
            dispatch_paired();
            break;
        case mission_state_t::spacejunk:
            dispatch_spacejunk();
            break;
        case mission_state_t::safehold:
            dispatch_safehold();
            break;
        case mission_state_t::manual:
            dispatch_manual();
            break;
        default:
            printf(debug_severity::error, "Master state not defined: %d\n", static_cast<unsigned int>(mode));
            mission_state_f.set(static_cast<unsigned int>(mission_state_t::safehold));
            break;
    }
}

void MissionManager::dispatch_startup() {
    set_mission_state(startup);
    set_adcs_state(startup);
    set_prop_mode(disabled);
    set_radio_mode(disabled);

    // Step 1. Wait for the deployment timer length.
    if (deployment_wait_elapsed_f.get() < deployment_wait) {
        deployment_wait_elapsed_f.set(deployment_wait_elapsed_f.get() + 1);
        return;
    }

    // Step 2. Check for hardware faults that would necessitate
    // going into an initialization hold
    if (check_hardware_faults()) {
        set_mission_state(initialization_hold);
    }

    // Step 3. If no hardware faults exist, go into detumble
    set_mission_state(detumble);
}

void MissionManager::dispatch_detumble() {
    set_mission_state(detumble);
    set_adcs_state(detumble);
    set_prop_mode(disabled);
    set_radio_mode(active);

    // Detumble until satellite angular rate is below an allowable threshold
    const f_vector_t ang_vel = adcs_ang_vel_fp->get();
    const lin::Vector3d ang_vel_vec {ang_vel[0], ang_vel[1], ang_vel[2]};
    const float ang_rate = lin::norm(ang_vel_vec);
    if (ang_rate <= adcs_min_stable_ang_rate_fp->get())
    {
        set_mission_state(standby);
    }
}

void MissionManager::dispatch_initialization_hold() {
    set_mission_state(initialization_hold);
    set_adcs_state(detumble);
    set_prop_mode(disabled);
    set_radio_mode(active);

    // Stay in this state until ground commands fix the satellite and
    // command the satellite out of this state.
}

void MissionManager::dispatch_follower() {
    set_satellite_designation(follower);
    set_mission_state(follower);
    set_adcs_state(point_standby);
    set_prop_mode(active);
    set_radio_mode(active);

    if (too_long_since_last_comms()) {
        set_satellite_designation(undecided);
        set_mission_state(standby);
    }

    if (distance_to_other_sat() < close_approach_trigger_dist) {
        set_mission_state(follower_close_approach);
    }
}

void MissionManager::dispatch_follower_close_approach() {
    set_satellite_designation(follower);
    set_mission_state(standby);
    set_adcs_state(point_docking);
    set_prop_mode(active);
    set_radio_mode(active);

    if (too_long_since_last_comms()) {
        set_satellite_designation(undecided);
        set_mission_state(standby);
    }

    if (distance_to_other_sat() < docking_trigger_dist) {
        set_mission_state(docking);
    }
}

void MissionManager::dispatch_standby() {
    set_mission_state(standby);
    set_adcs_state(point_standby);
    set_prop_mode(active); // Active in order to enable ground propulsion commands.
    set_radio_mode(active);

    satellite_designation_t const sat_designation =
        static_cast<satellite_designation_t>(sat_designation_f.get());

    if (sat_designation == satellite_designation_t::follower) {
        set_mission_state(follower);
    }
    else if (sat_designation == satellite_designation_t::leader
             && distance_to_other_sat() < close_approach_trigger_dist) {
        set_mission_state(leader_close_approach);
    }
    else {
        // The mission hasn't started yet. Let the satellite subsystems do their thing.
    }
}

void MissionManager::dispatch_leader_close_approach() {
    set_satellite_designation(leader);
    set_mission_state(standby);
    set_adcs_state(point_docking);
    set_prop_mode(active);
    set_radio_mode(active);

    if (too_long_since_last_comms()) {
        set_satellite_designation(undecided);
        set_mission_state(standby);
    }

    if (distance_to_other_sat() < docking_trigger_dist) {
        set_mission_state(docking);
    }
}

void MissionManager::dispatch_docking() {
    set_mission_state(docking);
    set_adcs_state(zero_torque);
    set_prop_mode(disabled);
    set_radio_mode(active);

    docking_config_cmd_f.set(true);
    if (docked_fp->get()) {
        set_mission_state(docked);
    }
}

void MissionManager::dispatch_docked() {
    set_mission_state(docked);
    set_adcs_state(zero_torque);
    set_prop_mode(disabled);
    set_radio_mode(active);

    // Mission has ended, so remove "follower" and "leader" designations.
    set_satellite_designation(undecided);
}

void MissionManager::dispatch_paired() {
    set_mission_state(paired);
    set_satellite_designation(undecided);

    // TODO initiate the modification of ADCS gains.
    // We could also do that inside the subsystem controller.
    set_mission_state(standby);
}

void MissionManager::dispatch_spacejunk() {
    set_mission_state(paired);
    set_adcs_state(zero_L);
    set_prop_mode(disabled);
    set_radio_mode(active);
    set_satellite_designation(undecided);

    // Wait for ground commands; do nothing more.
}

void MissionManager::dispatch_safehold() {
    set_mission_state(safehold);
    set_adcs_state(limited);
    set_prop_mode(disabled);
    set_radio_mode(active);

    // TODO auto-exits
}

void MissionManager::dispatch_manual() {
    // Allow spacecraft to be commandeered completely by test software or
    // the ground. This state should almost never be used by the ground.
    //
    // The only additional control it provides beyond the standby mode is
    // the ability to control attitude, which is a dangerous capability.
    // If this capability is desired, a better way to use it would be via
    // the Attitude Computer's point_manual mode.
}

double MissionManager::distance_to_other_sat() const {
    const d_vector_t dr = propagated_baseline_pos_fp->get();
    lin::Vector3d dr_vec = {dr[0], dr[1], dr[2]};
    return lin::norm(dr_vec);
}

bool MissionManager::too_long_since_last_comms() const {
    return control_cycle_count - last_checkin_cycle_fp->get() > max_radio_silence_duration;
}

#undef set_mission_state
#undef set_adcs_state
#undef set_prop_mode
#undef set_radio_state
#undef set_satellite_designation
