#include "MissionManager.hpp"
#include <lin/core.hpp>
#include <cmath>
#include <adcs/constants.hpp>
#include <common/constant_tracker.hpp>
#include <gnc/constants.hpp>
#include "SimpleFaultHandler.hpp"

// Declare static storage for constexpr variables
const constexpr double MissionManager::initial_detumble_safety_factor;
const constexpr double MissionManager::initial_close_approach_trigger_dist;
const constexpr double MissionManager::initial_docking_trigger_dist;
const constexpr unsigned int MissionManager::initial_docking_timeout_limit;
const constexpr unsigned int MissionManager::deployment_wait;
const constexpr unsigned char MissionManager::kill_switch_value;
const constexpr std::array<mission_state_t, 5> MissionManager::fault_responsive_states;
const constexpr std::array<mission_state_t, 7> MissionManager::fault_nonresponsive_states;

MissionManager::MissionManager(StateFieldRegistry &registry) 
    : TimedControlTask<void>(registry, "mission_ct"),
    detumble_safety_factor_f("detumble_safety_factor", Serializer<double>(0, 0.5, 7)),
    close_approach_trigger_dist_f("trigger_dist.close_approach", Serializer<double>(0, 5000, 13)),
    docking_trigger_dist_f("trigger_dist.docking", Serializer<double>(0, 100, 14)),
    docking_timeout_limit_f("docking_timeout_limit",
                            Serializer<unsigned int>(0, 2 * PAN::one_day_ccno, 6)),
    main_fault_handler(std::make_unique<MainFaultHandler>(registry)),
    adcs_state_f("adcs.state", Serializer<unsigned char>(10)),
    docking_config_cmd_f("docksys.config_cmd", Serializer<bool>()),
    enter_docking_cycle_f("docksys.enter_docking"),
    low_batt_fault_fp(FIND_FAULT(gomspace.low_batt.base)),
    adcs_functional_fault_fp(FIND_FAULT(adcs_monitor.functional_fault.base)),
    wheel1_adc_fault_fp(FIND_FAULT(adcs_monitor.wheel1_fault.base)),
    wheel2_adc_fault_fp(FIND_FAULT(adcs_monitor.wheel2_fault.base)),
    wheel3_adc_fault_fp(FIND_FAULT(adcs_monitor.wheel3_fault.base)),
    wheel_pot_fault_fp(FIND_FAULT(adcs_monitor.wheel_pot_fault.base)),
    pressurize_fail_fp(FIND_FAULT(prop.pressurize_fail.base)),
    mission_state_f("pan.state", Serializer<unsigned char>(12), 1),
    is_deployed_f("pan.deployed", Serializer<bool>(), 1000),
    deployment_wait_elapsed_f("pan.deployment.elapsed", Serializer<unsigned int>(15000), 500),
    sat_designation_f("pan.sat_designation", Serializer<unsigned char>(2)),
    enter_close_approach_ccno_f("pan.enter_close_approach_ccno"),
    kill_switch_f("pan.kill_switch", Serializer<unsigned char>(), 100)
{
    add_writable_field(detumble_safety_factor_f);
    add_writable_field(close_approach_trigger_dist_f);
    add_writable_field(docking_trigger_dist_f);
    add_writable_field(docking_timeout_limit_f);
    add_writable_field(adcs_state_f);
    add_writable_field(docking_config_cmd_f);
    add_internal_field(enter_docking_cycle_f);
    add_writable_field(mission_state_f);
    add_readable_field(is_deployed_f);
    add_readable_field(deployment_wait_elapsed_f);
    add_writable_field(sat_designation_f);
    add_internal_field(enter_close_approach_ccno_f);
    add_writable_field(kill_switch_f);

    bootcount_fp = find_readable_field<unsigned int>("pan.bootcount", __FILE__, __LINE__);
    bootcount_fp->set(bootcount_fp->get() + 1);

    static_cast<MainFaultHandler *>(main_fault_handler.get())->init();

    attitude_estimator_valid_fp = FIND_READABLE_FIELD(bool, attitude_estimator.valid);
    attitude_estimator_L_body_fp = FIND_READABLE_FIELD(lin::Vector3f, attitude_estimator.L_body);

    radio_state_fp = find_readable_field<unsigned char>("radio.state", __FILE__, __LINE__);
    last_checkin_cycle_fp = find_readable_field<unsigned int>("radio.last_comms_ccno", __FILE__, __LINE__);

    prop_state_fp = find_writable_field<unsigned int>("prop.state", __FILE__, __LINE__);

    rel_orbit_state_fp = FIND_READABLE_FIELD(unsigned char, rel_orbit.state);
    rel_orbit_rel_pos_fp = FIND_READABLE_FIELD(lin::Vector3d, rel_orbit.rel_pos);

    reset_fp = find_writable_field<bool>("gomspace.gs_reset_cmd", __FILE__, __LINE__);

    docked_fp = find_readable_field<bool>("docksys.docked", __FILE__, __LINE__);

    sph_dcdc_fp = find_writable_field<bool>("dcdc.SpikeDock_cmd", __FILE__, __LINE__);
    adcs_dcdc_fp = find_writable_field<bool>("dcdc.ADCSMotor_cmd", __FILE__, __LINE__);

    piksi_off_fp = find_writable_field<bool>("gomspace.piksi_off", __FILE__, __LINE__);
    piksi_powercycle_fp = find_writable_field<bool>("gomspace.power_cycle_output1_cmd", __FILE__, __LINE__);

    // Initialize a bunch of variables
    detumble_safety_factor_f.set(initial_detumble_safety_factor);
    close_approach_trigger_dist_f.set(initial_close_approach_trigger_dist);
    docking_trigger_dist_f.set(initial_docking_trigger_dist);
    docking_timeout_limit_f.set(initial_docking_timeout_limit);
    transition_to(mission_state_t::startup,
                  adcs_state_t::startup,
                  prop_state_t::disabled); // "Starting" transition
    docking_config_cmd_f.set(true);
    enter_docking_cycle_f.set(0);
    is_deployed_f.set(bootcount_fp->get() > 1);
    deployment_wait_elapsed_f.set(0);
    set(sat_designation_t::undecided);
}

void MissionManager::execute()
{
    mission_state_t state = static_cast<mission_state_t>(mission_state_f.get());

    // Step 1. Change state if faults exist.
    const fault_response_t fault_response = main_fault_handler->execute();

    if (!(state == mission_state_t::startup || state == mission_state_t::manual))
    {
        if (fault_response == fault_response_t::safehold)
        {
            transition_to(mission_state_t::safehold, adcs_state_t::startup, prop_state_t::disabled);
            set(sat_designation_t::undecided);
            return;
        }
        else if (fault_response == fault_response_t::standby && state != mission_state_t::safehold && state != mission_state_t::initialization_hold && state != mission_state_t::detumble && state != mission_state_t::standby)
        {
            transition_to(mission_state_t::standby, adcs_state_t::point_standby);
            set(sat_designation_t::undecided);
            return;
        }
    }

    // Step 2. Handle state.
    switch (state)
    {
    case mission_state_t::startup:
        dispatch_startup();
        break;
    case mission_state_t::detumble:
        dispatch_detumble();
        break;
    case mission_state_t::initialization_hold:
        dispatch_initialization_hold();
        break;
    case mission_state_t::standby:
        dispatch_standby();
        break;
    case mission_state_t::follower:
        dispatch_follower();
        break;
    case mission_state_t::leader:
        dispatch_leader();
        break;
    case mission_state_t::follower_close_approach:
        dispatch_follower_close_approach();
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
    case mission_state_t::safehold:
        dispatch_safehold();
        break;
    case mission_state_t::manual:
        dispatch_manual();
        break;
    default:
        printf(debug_severity::error, "Master state not defined: %d\n", static_cast<unsigned char>(state));
        transition_to(mission_state_t::safehold, adcs_state_t::startup);
        break;
    }
}

bool MissionManager::check_adcs_hardware_faults() const
{
    return adcs_functional_fault_fp->is_faulted() || wheel1_adc_fault_fp->is_faulted() || wheel2_adc_fault_fp->is_faulted() || wheel3_adc_fault_fp->is_faulted() || wheel_pot_fault_fp->is_faulted();
}

void MissionManager::dispatch_startup()
{
    // Step 0. If kill switch flag is set, shuts down radio connection
    if (kill_switch_f.get() == kill_switch_value)
    {
        while (true)
        {
            //This will cause gomspace to reboot the spacecraft and we will get stuck in this loop forever
        }
    }

    // Step 1. Wait for the deployment timer length. Skip if bootcount > 1.
    if (bootcount_fp->get() == 1) {
        if (deployment_wait_elapsed_f.get() < deployment_wait)
        {
            deployment_wait_elapsed_f.set(deployment_wait_elapsed_f.get() + 1);
            return;
        }
    }

    // Step 2. Once we've complete the deployment wait, if any, we want to turn
    // the radio on if it isn't already and enable to piksi. We also check for
    // hardware faults that would necessitate going into an initialization hold.
    // If such faults exist, go into initialization hold, otherwise detumble.
    piksi_off_fp->set(false);
    if (radio_state_fp->get() == static_cast<unsigned char>(radio_state_t::disabled))
    {
        set(radio_state_t::config);
    }
    if (check_adcs_hardware_faults())
    {
        transition_to(mission_state_t::initialization_hold,
                      adcs_state_t::detumble,
                      prop_state_t::idle);
    }
    else
    {
        is_deployed_f.set(true);
        transition_to(mission_state_t::detumble,
                      adcs_state_t::detumble,
                      prop_state_t::idle);
    }
}

void MissionManager::dispatch_detumble()
{
    if (attitude_estimator_valid_fp->get())
    {
        // Detumble until satellite angular rate is below an allowable threshold
        const float momentum = lin::fro(attitude_estimator_L_body_fp->get());
        const float threshold = adcs::rwa::max_speed_read * adcs::rwa::moment_of_inertia * detumble_safety_factor_f.get();

        if (momentum <= threshold * threshold) // Save a sqrt call and use fro norm
        {
            if (!adcs_dcdc_fp->get()) // cause a cycle where DCDC is turned on then wheels turn on
                adcs_dcdc_fp->set(true);
            else
                transition_to(mission_state_t::standby, adcs_state_t::point_standby);
            // dcdc will be reasserted to true but that's ok
        }
    }
}

void MissionManager::dispatch_initialization_hold()
{
    // Stay in this state until ground commands fix the satellite and
    // command the satellite out of this state.
}

void MissionManager::dispatch_standby()
{
    sat_designation_t const sat_designation =
        static_cast<sat_designation_t>(sat_designation_f.get());

    if (sat_designation == sat_designation_t::follower)
    {
        transition_to(mission_state_t::follower,
                      adcs_state_t::point_standby);
    }
    else if (sat_designation == sat_designation_t::leader)
    {
        transition_to(mission_state_t::leader,
                      adcs_state_t::point_standby);
    }
    else
    {
        // The mission hasn't started yet. Let the satellite subsystems do their thing.
    }
}

void MissionManager::dispatch_follower()
{
    if (distance_to_other_sat() < close_approach_trigger_dist_f.get())
    {
        transition_to(mission_state_t::follower_close_approach,
                      adcs_state_t::point_docking);
    }
}

void MissionManager::dispatch_leader()
{
    if (distance_to_other_sat() < close_approach_trigger_dist_f.get())
    {
        transition_to(mission_state_t::leader_close_approach,
                      adcs_state_t::point_docking);
    }
}

void MissionManager::dispatch_follower_close_approach()
{
    docking_config_cmd_f.set(true);

    if (distance_to_other_sat() < docking_trigger_dist_f.get())
    {
        transition_to(mission_state_t::docking,
                      adcs_state_t::zero_torque);
    }
}

void MissionManager::dispatch_leader_close_approach()
{
    docking_config_cmd_f.set(true);

    if (distance_to_other_sat() < docking_trigger_dist_f.get())
    {
        transition_to(mission_state_t::docking,
                      adcs_state_t::zero_torque);
    }
}

/**
 * @brief This flag checks if we've set the state field called docking_entry_ccno,
 * which indicates the control cycle # at which we entered the docking state.
 * This state field is used by PiksiFaultHandler to know if we've been lacking
 * CDGPS for too long.
 */
static bool have_set_docking_entry_ccno = false;

void MissionManager::dispatch_docking()
{
    docking_config_cmd_f.set(true);
    if (!have_set_docking_entry_ccno)
    {
        enter_docking_cycle_f.set(control_cycle_count);
        have_set_docking_entry_ccno = true;
    }

    if (docked_fp->get())
    {
        have_set_docking_entry_ccno = false;
        transition_to(mission_state_t::docked,
                      adcs_state_t::zero_torque);

        // Mission has ended, so remove "follower" and "leader" designations.
        set(sat_designation_t::undecided);
    }
    else if (too_long_in_docking())
    {
        have_set_docking_entry_ccno = false;
        transition_to(mission_state_t::standby,
                      adcs_state_t::point_standby);
        // ADCS dcdc should already be on, no need to re-assert
    }
}

void MissionManager::dispatch_docked()
{
    // Do nothing. Wait for a ground command to separate states.
}

void MissionManager::dispatch_safehold()
{
    if (control_cycle_count - safehold_begin_ccno >= PAN::one_day_ccno)
        reset_fp->set(true);
}

void MissionManager::dispatch_manual()
{
    // Do nothing.
}

double MissionManager::distance_to_other_sat() const
{
    return rel_orbit_state_fp->get() ? lin::norm(rel_orbit_rel_pos_fp->get()) : gnc::constant::nan;
}

bool MissionManager::too_long_in_docking() const
{
    const unsigned int cycles_since_enter_docking = control_cycle_count - enter_docking_cycle_f.get();
    return cycles_since_enter_docking > docking_timeout_limit_f.get();
}

void MissionManager::set(mission_state_t state)
{
    if (state == mission_state_t::safehold)
    {
        safehold_begin_ccno = control_cycle_count;
    }
    if (state == mission_state_t::leader_close_approach || state == mission_state_t::follower_close_approach)
    {
        enter_close_approach_ccno_f.set(control_cycle_count);
    }
    mission_state_f.set(static_cast<unsigned char>(state));
}

void MissionManager::set(adcs_state_t state)
{
    adcs_state_f.set(static_cast<unsigned char>(state));
}

void MissionManager::set(prop_state_t state)
{
    prop_state_fp->set(static_cast<unsigned char>(state));
}

void MissionManager::set(radio_state_t state)
{
    radio_state_fp->set(static_cast<unsigned char>(state));
}

void MissionManager::set(sat_designation_t designation)
{
    sat_designation_f.set(static_cast<unsigned char>(designation));
}

void MissionManager::transition_to(mission_state_t mission_state,
                                   adcs_state_t adcs_state)
{
    if (mission_state == mission_state_t::safehold)
    {
        adcs_dcdc_fp->set(false);
    }
    // all other transitions shall leave the DCDC's alone

    set(mission_state);
    set(adcs_state);
}

void MissionManager::transition_to(mission_state_t mission_state,
                                   adcs_state_t adcs_state,
                                   prop_state_t prop_state)
{
    if (prop_state == prop_state_t::disabled)
    {
        sph_dcdc_fp->set(false);
    }
    else
    {
        sph_dcdc_fp->set(true);
    }

    transition_to(mission_state, adcs_state);
    set(prop_state);
}
