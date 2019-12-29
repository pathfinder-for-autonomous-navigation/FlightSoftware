#include "MissionManager.hpp"
#include <lin.hpp>
#include <cmath>

MissionManager::MissionManager(StateFieldRegistry& registry, unsigned int offset) :
    TimedControlTask<void>(registry, "mission_ct", offset),
    adcs_state_f("adcs.state", Serializer<unsigned char>(10)),
    docking_config_cmd_f("docksys.config_cmd", Serializer<bool>()),
    mission_state_f("pan.state", Serializer<unsigned char>(14)),
    is_deployed_f("pan.deployed", Serializer<bool>()),
    deployment_wait_elapsed_f("pan.deployment.elapsed", Serializer<unsigned int>(0, 15000, 32)),
    sat_designation_f("pan.sat_designation", Serializer<unsigned char>(2))
{
    add_writable_field(adcs_state_f);
    add_writable_field(docking_config_cmd_f);
    add_writable_field(mission_state_f);
    add_readable_field(is_deployed_f);
    add_readable_field(deployment_wait_elapsed_f);
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
    set(mission_state_t::startup);
    set(adcs_state_t::startup);
    set(prop_mode_t::disabled);
    set(radio_mode_t::disabled);
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
        bool faulted = check_hardware_faults();
        if (faulted) set(mission_state_t::safehold);
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
        case mission_state_t::leader:
            dispatch_leader();
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
            set(mission_state_t::safehold);
            break;
    }
}

void MissionManager::dispatch_startup() {
    set(mission_state_t::startup);
    set(adcs_state_t::startup);
    set(prop_mode_t::disabled);
    set(radio_mode_t::disabled);

    // Step 1. Wait for the deployment timer length.
    if (deployment_wait_elapsed_f.get() < deployment_wait) {
        deployment_wait_elapsed_f.set(deployment_wait_elapsed_f.get() + 1);
        return;
    }

    // Step 2. Check for hardware faults that would necessitate
    // going into an initialization hold
    if (check_hardware_faults()) {
        set(mission_state_t::initialization_hold);
    }

    // Step 3. If no hardware faults exist, go into detumble
    set(mission_state_t::detumble);
}

void MissionManager::dispatch_detumble() {
    set(mission_state_t::detumble);
    set(adcs_state_t::detumble);
    set(prop_mode_t::disabled);
    set(radio_mode_t::active);

    // Detumble until satellite angular rate is below an allowable threshold
    const f_vector_t ang_vel = adcs_ang_vel_fp->get();
    const lin::Vector3f ang_vel_vec {ang_vel[0], ang_vel[1], ang_vel[2]};
    const float ang_rate = lin::norm(ang_vel_vec);
    if (ang_rate <= adcs_min_stable_ang_rate_fp->get())
    {
        set(adcs_state_t::point_standby);
        set(mission_state_t::standby);
    }
}

void MissionManager::dispatch_initialization_hold() {
    set(mission_state_t::initialization_hold);
    set(adcs_state_t::detumble);
    set(prop_mode_t::disabled);
    set(radio_mode_t::active);

    // Stay in this state until ground commands fix the satellite and
    // command the satellite out of this state.
}

void MissionManager::dispatch_follower() {
    set(sat_designation_t::follower);
    set(mission_state_t::follower);
    set(adcs_state_t::point_standby);
    set(prop_mode_t::active);
    set(radio_mode_t::active);

    if (too_long_since_last_comms()) {
        set(sat_designation_t::undecided);
        set(mission_state_t::standby);
    }

    if (distance_to_other_sat() < close_approach_trigger_dist) {
        set(mission_state_t::follower_close_approach);
    }
}

void MissionManager::dispatch_follower_close_approach() {
    set(sat_designation_t::follower);
    set(mission_state_t::follower_close_approach);
    set(adcs_state_t::point_docking);
    set(prop_mode_t::active);
    set(radio_mode_t::active);

    if (too_long_since_last_comms()) {
        set(sat_designation_t::undecided);
        set(adcs_state_t::point_standby);
        set(mission_state_t::standby);
    }

    if (distance_to_other_sat() < docking_trigger_dist) {
        set(mission_state_t::docking);
    }
}

void MissionManager::dispatch_standby() {
    set(mission_state_t::standby);
    set(prop_mode_t::active); // Active in order to enable ground propulsion commands.
    set(radio_mode_t::active);

    sat_designation_t const sat_designation =
        static_cast<sat_designation_t>(sat_designation_f.get());

    if (sat_designation == sat_designation_t::follower) {
        set(adcs_state_t::set_singlesat_gains);
        set(mission_state_t::follower);
    }
    else if (sat_designation == sat_designation_t::leader) {
        set(adcs_state_t::set_singlesat_gains);
        set(mission_state_t::leader);
    }
    else {
        // The mission hasn't started yet. Let the satellite subsystems do their thing.
    }
}

void MissionManager::dispatch_leader() {
    set(sat_designation_t::leader);
    set(mission_state_t::leader);
    set(adcs_state_t::point_standby);
    set(prop_mode_t::disabled);
    set(radio_mode_t::active);

    if (too_long_since_last_comms()) {
        set(sat_designation_t::undecided);
        set(adcs_state_t::point_standby);
        set(mission_state_t::standby);
    }

    if (distance_to_other_sat() < close_approach_trigger_dist) {
        set(mission_state_t::leader_close_approach);
    }
}

void MissionManager::dispatch_leader_close_approach() {
    set(sat_designation_t::leader);
    set(mission_state_t::leader_close_approach);
    set(adcs_state_t::point_docking);
    set(prop_mode_t::active);
    set(radio_mode_t::active);

    if (too_long_since_last_comms()) {
        set(sat_designation_t::undecided);
        set(adcs_state_t::point_standby);
        set(mission_state_t::standby);
    }

    if (distance_to_other_sat() < docking_trigger_dist) {
        set(mission_state_t::docking);
    }
}

void MissionManager::dispatch_docking() {
    set(mission_state_t::docking);
    set(adcs_state_t::zero_torque);
    set(prop_mode_t::disabled);
    set(radio_mode_t::active);

    docking_config_cmd_f.set(true);
    if (docked_fp->get()) {
        set(mission_state_t::docked);
    }
}

void MissionManager::dispatch_docked() {
    set(mission_state_t::docked);
    set(adcs_state_t::zero_torque);
    set(prop_mode_t::disabled);
    set(radio_mode_t::active);

    // Mission has ended, so remove "follower" and "leader" designations.
    set(sat_designation_t::undecided);
}

void MissionManager::dispatch_paired() {
    set(mission_state_t::paired);
    set(sat_designation_t::undecided);
    set(adcs_state_t::set_paired_gains);
    set(mission_state_t::standby);
}

void MissionManager::dispatch_spacejunk() {
    set(mission_state_t::spacejunk);
    set(adcs_state_t::zero_L);
    set(prop_mode_t::disabled);
    set(radio_mode_t::active);
    set(sat_designation_t::undecided);

    // Wait for ground commands; do nothing more.
}

void MissionManager::dispatch_safehold() {
    set(mission_state_t::safehold);
    set(adcs_state_t::limited);
    set(prop_mode_t::disabled);
    set(radio_mode_t::active);

    // TODO auto-exits
}

void MissionManager::dispatch_manual() {
    // Do nothing.
}


double MissionManager::distance_to_other_sat() const {
    const d_vector_t dr = propagated_baseline_pos_fp->get();
    if (std::isnan(dr[0])) return dr[0];
    lin::Vector3d dr_vec = {dr[0], dr[1], dr[2]};
    return lin::norm(dr_vec);
}

bool MissionManager::too_long_since_last_comms() const {
    const unsigned int cycles_since_last_comms = control_cycle_count - last_checkin_cycle_fp->get();
    return cycles_since_last_comms > max_radio_silence_duration;
}

void MissionManager::set(mission_state_t state) {
    mission_state_f.set(static_cast<unsigned int>(state));
}

void MissionManager::set(adcs_state_t state) {
    adcs_state_f.set(static_cast<unsigned int>(state));
}

void MissionManager::set(prop_mode_t mode) {
    prop_mode_fp->set(static_cast<unsigned int>(mode));
}

void MissionManager::set(radio_mode_t mode) {
    radio_mode_fp->set(static_cast<unsigned int>(mode));
}

void MissionManager::set(sat_designation_t designation) {
    sat_designation_f.set(static_cast<unsigned int>(designation));
}
