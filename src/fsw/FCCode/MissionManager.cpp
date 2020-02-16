#include "MissionManager.hpp"
#include <lin.hpp>
#include <cmath>
#include <adcs/constants.hpp>

// Declare static storage for constexpr variables
const constexpr double MissionManager::initial_detumble_safety_factor;
const constexpr double MissionManager::initial_close_approach_trigger_dist;
const constexpr double MissionManager::initial_docking_trigger_dist;
const constexpr unsigned int MissionManager::initial_max_radio_silence_duration;
const constexpr unsigned int MissionManager::deployment_wait;
const constexpr std::array<mission_state_t, 5> MissionManager::fault_responsive_states;
const constexpr std::array<mission_state_t, 7> MissionManager::fault_nonresponsive_states;

MissionManager::MissionManager(StateFieldRegistry& registry, unsigned int offset) :
    TimedControlTask<void>(registry, "mission_ct", offset),
    detumble_safety_factor_f("detumble_safety_factor", Serializer<double>(0, 1, 10)),
    close_approach_trigger_dist_f("trigger_dist.close_approach", Serializer<double>(0, 10000, 14)),
    docking_trigger_dist_f("trigger_dist.docking", Serializer<double>(0, 100, 10)),
    max_radio_silence_duration_f("max_radio_silence",
        Serializer<unsigned int>(2 * PAN::one_day_ccno)),
    adcs_state_f("adcs.state", Serializer<unsigned char>(10)),
    docking_config_cmd_f("docksys.config_cmd", Serializer<bool>()),
    mission_state_f("pan.state", Serializer<unsigned char>(12)),
    is_deployed_f("pan.deployed", Serializer<bool>()),
    deployment_wait_elapsed_f("pan.deployment.elapsed", Serializer<unsigned int>(0, 15000, 32)),
    sat_designation_f("pan.sat_designation", Serializer<unsigned char>(2))
{
    add_writable_field(detumble_safety_factor_f);
    add_writable_field(close_approach_trigger_dist_f);
    add_writable_field(docking_trigger_dist_f);
    add_writable_field(max_radio_silence_duration_f);
    add_writable_field(adcs_state_f);
    add_writable_field(docking_config_cmd_f);
    add_writable_field(mission_state_f);
    add_readable_field(is_deployed_f);
    add_readable_field(deployment_wait_elapsed_f);
    add_writable_field(sat_designation_f);

    adcs_paired_fp = find_writable_field<bool>("adcs.paired", __FILE__, __LINE__);
    adcs_ang_momentum_fp = find_internal_field<lin::Vector3f>("attitude_estimator.h_body", __FILE__, __LINE__);

    radio_state_fp = find_internal_field<unsigned char>("radio.state", __FILE__, __LINE__);
    last_checkin_cycle_fp = find_internal_field<unsigned int>("radio.last_comms_ccno", __FILE__, __LINE__);

    prop_state_fp = find_readable_field<unsigned char>("prop.state", __FILE__, __LINE__);

    propagated_baseline_pos_fp = find_readable_field<d_vector_t>("orbit.baseline_pos", __FILE__, __LINE__);

    reboot_fp = find_writable_field<bool>("gomspace.gs_reboot_cmd", __FILE__, __LINE__);

    docked_fp = find_readable_field<bool>("docksys.docked", __FILE__, __LINE__);

    low_batt_fault_fp = static_cast<Fault*>(
            find_writable_field<bool>("gomspace.low_batt", __FILE__, __LINE__));
    adcs_functional_fault_fp = static_cast<Fault*>(
            find_writable_field<bool>("adcs_monitor.functional_fault", __FILE__, __LINE__));
    wheel1_adc_fault_fp = static_cast<Fault*>(
            find_writable_field<bool>("adcs_monitor.wheel1_fault", __FILE__, __LINE__));
    wheel2_adc_fault_fp = static_cast<Fault*>(
            find_writable_field<bool>("adcs_monitor.wheel2_fault", __FILE__, __LINE__));
    wheel3_adc_fault_fp = static_cast<Fault*>(
            find_writable_field<bool>("adcs_monitor.wheel3_fault", __FILE__, __LINE__));
    wheel_pot_fault_fp = static_cast<Fault*>(
            find_writable_field<bool>("adcs_monitor.wheel_pot_fault", __FILE__, __LINE__));
    failed_pressurize_fp = static_cast<Fault*>(
            find_writable_field<bool>("prop.failed_pressurize", __FILE__, __LINE__));

    // Initialize a bunch of variables
    detumble_safety_factor_f.set(initial_detumble_safety_factor);
    close_approach_trigger_dist_f.set(initial_close_approach_trigger_dist);
    docking_trigger_dist_f.set(initial_docking_trigger_dist);
    max_radio_silence_duration_f.set(initial_max_radio_silence_duration);
    transition_to_state(mission_state_t::startup,
        adcs_state_t::startup,
        prop_state_t::disabled); // "Starting" transition
    docking_config_cmd_f.set(true);
    is_deployed_f.set(false);
    deployment_wait_elapsed_f.set(0);
    set(sat_designation_t::undecided);
}

bool MissionManager::check_adcs_hardware_faults() const {
    return  adcs_functional_fault_fp->is_faulted()
            || wheel1_adc_fault_fp->is_faulted() 
            || wheel2_adc_fault_fp->is_faulted()
            || wheel3_adc_fault_fp->is_faulted()
            || wheel_pot_fault_fp->is_faulted();
}

void MissionManager::execute() {
    mission_state_t state = static_cast<mission_state_t>(mission_state_f.get());

    // Step 1. Disable radio if in startup.
    if (state == mission_state_t::startup) {
        set(radio_state_t::disabled);
    }

    // Step 2. Change state if faults exist.
    bool is_fault_responsive_state = false;
    for(mission_state_t fault_responsive_state : fault_responsive_states) {
        if (state == fault_responsive_state) {
            is_fault_responsive_state = true;
            break;
        }
    }
    if (is_fault_responsive_state) {
        const bool prop_depressurized = failed_pressurize_fp->is_faulted();
        const bool power_faulted = low_batt_fault_fp->is_faulted();
        const bool adcs_faulted = check_adcs_hardware_faults();

        // Note: faults that cause safehold should be checked before faults that
        // merely cause a transition back to standby.
        if (power_faulted || adcs_faulted) {
            transition_to_state(mission_state_t::safehold,
                adcs_state_t::startup,
                prop_state_t::disabled);
            return;
        }
        else if (prop_depressurized) {
            transition_to_state(mission_state_t::standby,
                adcs_state_t::point_standby,
                prop_state_t::idle);
            return;
        } 
    }

    // Step 3. Handle state.
    switch(state) {
        case mission_state_t::startup:                    dispatch_startup();                    break;
        case mission_state_t::detumble:                   dispatch_detumble();                   break;
        case mission_state_t::initialization_hold:        dispatch_initialization_hold();        break;
        case mission_state_t::standby:                    dispatch_standby();                    break;
        case mission_state_t::follower:                   dispatch_follower();                   break;
        case mission_state_t::leader:                     dispatch_leader();                     break;
        case mission_state_t::follower_close_approach:    dispatch_follower_close_approach();    break;
        case mission_state_t::leader_close_approach:      dispatch_leader_close_approach();      break;
        case mission_state_t::docking:                    dispatch_docking();                    break;
        case mission_state_t::docked:                     dispatch_docked();                     break;
        case mission_state_t::safehold:                   dispatch_safehold();                   break;
        case mission_state_t::manual:                     dispatch_manual();                     break;
        default:
            printf(debug_severity::error, "Master state not defined: %d\n", static_cast<unsigned char>(state));
            transition_to_state(mission_state_t::safehold, adcs_state_t::startup, prop_state_t::disabled);
            break;
    }
}

void MissionManager::dispatch_startup() {
    // Step 1. Wait for the deployment timer length.
    if (deployment_wait_elapsed_f.get() < deployment_wait) {
        deployment_wait_elapsed_f.set(deployment_wait_elapsed_f.get() + 1);
        return;
    }

    // Step 2. Turn radio on, and check for hardware faults that would necessitate
    // going into an initialization hold. If faults exist, go into
    // initialization hold, otherwise detumble.
    set(radio_state_t::wait);
    if (check_adcs_hardware_faults()) {
        transition_to_state(mission_state_t::initialization_hold,
            adcs_state_t::detumble,
            prop_state_t::disabled);
    }
    else {
        transition_to_state(mission_state_t::detumble,
            adcs_state_t::detumble,
            prop_state_t::disabled);
    }
}

void MissionManager::dispatch_detumble() {
    // Detumble until satellite angular rate is below an allowable threshold
    const float momentum = lin::norm(adcs_ang_momentum_fp->get());
    const float threshold = adcs::rwa::max_speed_read * adcs::rwa::moment_of_inertia * detumble_safety_factor_f.get();
    if (momentum <= threshold)
    {
        transition_to_state(mission_state_t::standby,
            adcs_state_t::point_standby,
            prop_state_t::idle);
    }
}

void MissionManager::dispatch_initialization_hold() {
    // Stay in this state until ground commands fix the satellite and
    // command the satellite out of this state.
}

void MissionManager::dispatch_standby() {
    sat_designation_t const sat_designation =
        static_cast<sat_designation_t>(sat_designation_f.get());

    if (sat_designation == sat_designation_t::follower) {
        adcs_paired_fp->set(false);
        transition_to_state(mission_state_t::follower,
            adcs_state_t::point_standby,
            prop_state_t::idle);
    }
    else if (sat_designation == sat_designation_t::leader) {
        adcs_paired_fp->set(false);
        transition_to_state(mission_state_t::leader,
            adcs_state_t::point_standby,
            prop_state_t::idle);
    }
    else {
        // The mission hasn't started yet. Let the satellite subsystems do their thing.
    }
}

void MissionManager::dispatch_follower() {
    if (distance_to_other_sat() < close_approach_trigger_dist_f.get()) {
        transition_to_state(mission_state_t::follower_close_approach,
            adcs_state_t::point_docking);
    }
    else if (too_long_since_last_comms()) {
        set(sat_designation_t::undecided);
        transition_to_state(mission_state_t::standby,
            adcs_state_t::point_standby,
            prop_state_t::idle);
    }
}

void MissionManager::dispatch_leader() {
    if (distance_to_other_sat() < close_approach_trigger_dist_f.get()) {
        transition_to_state(mission_state_t::leader_close_approach,
            adcs_state_t::point_docking,
            prop_state_t::disabled);
    }
    else if (too_long_since_last_comms()) {
        set(sat_designation_t::undecided);
        transition_to_state(mission_state_t::standby,
            adcs_state_t::point_standby,
            prop_state_t::idle);
    }
}

void MissionManager::dispatch_follower_close_approach() {
    docking_config_cmd_f.set(true);

    if (distance_to_other_sat() < docking_trigger_dist_f.get()) {
        transition_to_state(mission_state_t::docking,
            adcs_state_t::zero_torque,
            prop_state_t::disabled);
    }
    else if (too_long_since_last_comms()) {
        set(sat_designation_t::undecided);
        transition_to_state(mission_state_t::standby,
            adcs_state_t::point_standby,
            prop_state_t::idle);
    }
}

void MissionManager::dispatch_leader_close_approach() {
    docking_config_cmd_f.set(true);

    if (distance_to_other_sat() < docking_trigger_dist_f.get()) {
        transition_to_state(mission_state_t::docking,
            adcs_state_t::zero_torque,
            prop_state_t::disabled);
    }
    else if (too_long_since_last_comms()) {
        set(sat_designation_t::undecided);
        transition_to_state(mission_state_t::standby,
            adcs_state_t::point_standby,
            prop_state_t::idle);
    }
}

void MissionManager::dispatch_docking() {
    docking_config_cmd_f.set(true);

    if (docked_fp->get()) {
        transition_to_state(mission_state_t::docked,
            adcs_state_t::zero_torque,
            prop_state_t::disabled);

        // Mission has ended, so remove "follower" and "leader" designations.
        set(sat_designation_t::undecided);
    }
}

void MissionManager::dispatch_docked() {
    // Do nothing. Wait for a ground command to separate states.
}

void MissionManager::dispatch_safehold() {
    if (control_cycle_count - safehold_begin_ccno >= PAN::one_day_ccno)
        reboot_fp->set(true);
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
    return cycles_since_last_comms > max_radio_silence_duration_f.get();
}

void MissionManager::set(mission_state_t state) {
    if (state == mission_state_t::safehold) {
        safehold_begin_ccno = control_cycle_count;
    }
    mission_state_f.set(static_cast<unsigned char>(state));
}

void MissionManager::set(adcs_state_t state) {
    adcs_state_f.set(static_cast<unsigned char>(state));
}

void MissionManager::set(prop_state_t state) {
    prop_state_fp->set(static_cast<unsigned char>(state));
}

void MissionManager::set(radio_state_t state) {
    radio_state_fp->set(static_cast<unsigned char>(state));
}

void MissionManager::set(sat_designation_t designation) {
    sat_designation_f.set(static_cast<unsigned char>(designation));
}

void MissionManager::transition_to_state(mission_state_t mission_state,
        adcs_state_t adcs_state)
{
    set(mission_state);
    set(adcs_state);
}

void MissionManager::transition_to_state(mission_state_t mission_state,
        adcs_state_t adcs_state,
        prop_state_t prop_state)
{
    set(mission_state);
    set(adcs_state);
    set(prop_state);
}
