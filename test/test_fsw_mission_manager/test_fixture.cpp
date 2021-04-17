#include "test_fixture.hpp"
#include "../test_fsw_fault_handler/FaultHandlerMachineMock.hpp"
#include <fsw/FCCode/Estimators/rel_orbit_state_t.enum>
#include <gnc/constants.hpp>
#include <limits>

TestFixture::TestFixture(mission_state_t initial_state, unsigned int bootcount) : registry()
{
    attitude_estimator_valid_fp = registry.create_readable_field<bool>(
        "attitude_estimator.valid");
    attitude_estimator_L_body_fp = registry.create_readable_lin_vector_field<float>(
        "attitude_estimator.L_body", 0, 1, 1);

    radio_state_fp = registry.create_readable_field<unsigned char>("radio.state");
    last_checkin_cycle_fp = registry.create_readable_field<unsigned int>(
        "radio.last_comms_ccno");

    prop_state_fp = registry.create_writable_field<unsigned int>("prop.state", 6);

    rel_orbit_state_fp = registry.create_readable_field<unsigned char>(
        "rel_orbit.state", 3);
    rel_orbit_rel_pos_fp = registry.create_readable_lin_vector_field<double>(
        "rel_orbit.rel_pos", 0, 100000, 100);

    reset_fp = registry.create_writable_field<bool>("gomspace.gs_reset_cmd");
    power_cycle_radio_fp = registry.create_writable_field<bool>("gomspace.power_cycle_output3_cmd");

    docked_fp = registry.create_readable_field<bool>("docksys.docked");

    bootcount_fp = registry.create_readable_field<unsigned int, 1000>("pan.bootcount"); 
    bootcount_fp->set(bootcount);

    low_batt_fault_fp = registry.create_fault("gomspace.low_batt", 1);
    adcs_functional_fault_fp = registry.create_fault("adcs_monitor.functional_fault", 1);
    wheel1_adc_fault_fp = registry.create_fault("adcs_monitor.wheel1_fault", 1);
    wheel2_adc_fault_fp = registry.create_fault("adcs_monitor.wheel2_fault", 1);
    wheel3_adc_fault_fp = registry.create_fault("adcs_monitor.wheel3_fault", 1);
    wheel_pot_fault_fp = registry.create_fault("adcs_monitor.wheel_pot_fault", 1);
    pressurize_fail_fp = registry.create_fault("prop.pressurize_fail", 1);
    overpressured_fp = registry.create_fault("prop.overpressured", 1);
    attitude_estimator_fp = registry.create_fault("attitude_estimator.fault", 1);

    sph_dcdc_fp = registry.create_writable_field<bool>("dcdc.SpikeDock_cmd");
    adcs_dcdc_fp = registry.create_writable_field<bool>("dcdc.ADCSMotor_cmd");

    piksi_state_fp = registry.create_readable_field<unsigned char>("piksi.state");
    piksi_off_fp = registry.create_writable_field<bool>("gomspace.piksi_off");
    piksi_powercycle_fp = registry.create_writable_field<bool>("gomspace.power_cycle_output1_cmd");

    // Initialize these variables
    attitude_estimator_valid_fp->set(false);
    radio_state_fp->set(static_cast<unsigned char>(radio_state_t::disabled));
    last_checkin_cycle_fp->set(0);
    prop_state_fp->set(static_cast<unsigned int>(prop_state_t::disabled));
    rel_orbit_state_fp->set(static_cast<unsigned char>(rel_orbit_state_t::invalid));
    reset_fp->set(false);
    power_cycle_radio_fp->set(false);
    docked_fp->set(false);

    mission_manager = std::make_unique<MissionManager>(registry, 0);

    // Check that mission manager creates its expected fields
    detumble_safety_factor_fp = registry.find_writable_field_t<double>("detumble_safety_factor");
    close_approach_trigger_dist_fp = registry.find_writable_field_t<double>("trigger_dist.close_approach");
    docking_trigger_dist_fp = registry.find_writable_field_t<double>("trigger_dist.docking");
    docking_timeout_limit_fp = registry.find_writable_field_t<unsigned int>("docking_timeout_limit");
    adcs_state_fp = registry.find_writable_field_t<unsigned char>("adcs.state");
    docking_config_cmd_fp = registry.find_writable_field_t<bool>("docksys.config_cmd");
    mission_state_fp = registry.find_writable_field_t<unsigned char>("pan.state");
    is_deployed_fp = registry.find_readable_field_t<bool>("pan.deployed");
    deployment_wait_elapsed_fp = registry.find_readable_field_t<unsigned int>(
        "pan.deployment.elapsed");
    sat_designation_fp = registry.find_writable_field_t<unsigned char>("pan.sat_designation");


    // Replace fault handler with a mock.
    mission_manager->main_fault_handler = std::make_unique<FaultHandlerMachineMock>(registry);

    // Set initial state.
    mission_state_fp->set(static_cast<unsigned char>(initial_state));
}

// Set and assert functions for various mission states.

void TestFixture::set(fault_response_t response)
{
    static_cast<FaultHandlerMachineMock *>(
        mission_manager->main_fault_handler.get())
        ->set(response);
}

void TestFixture::set(mission_state_t state)
{
    mission_manager->set(state);
}

void TestFixture::set(adcs_state_t state)
{
    adcs_state_fp->set(static_cast<unsigned char>(state));
}

void TestFixture::set(prop_state_t state)
{
    prop_state_fp->set(static_cast<unsigned int>(state));
}

void TestFixture::set(radio_state_t state)
{
    radio_state_fp->set(static_cast<unsigned char>(state));
}

void TestFixture::set(sat_designation_t designation)
{
    sat_designation_fp->set(static_cast<unsigned char>(designation));
}

void TestFixture::check(mission_state_t state) const
{
    TEST_ASSERT_EQUAL_MESSAGE(static_cast<unsigned char>(state), mission_state_fp->get(),
                              "For mission state.");
}

void TestFixture::check(adcs_state_t state) const
{
    TEST_ASSERT_EQUAL_MESSAGE(static_cast<unsigned char>(state), adcs_state_fp->get(),
                              "For ADCS state.");
}

void TestFixture::check(prop_state_t state) const
{
    TEST_ASSERT_EQUAL_MESSAGE(static_cast<unsigned int>(state), prop_state_fp->get(),
                              "For propulsion state.");
}

void TestFixture::check(radio_state_t state) const
{
    TEST_ASSERT_EQUAL_MESSAGE(static_cast<unsigned char>(state), radio_state_fp->get(),
                              "For radio state.");
}

void TestFixture::check(sat_designation_t designation) const
{
    TEST_ASSERT_EQUAL_MESSAGE(static_cast<unsigned char>(designation), sat_designation_fp->get(),
                              "For satellite designation.");
}

void TestFixture::check_sph_dcdc_on(bool on) const
{
    TEST_ASSERT(sph_dcdc_fp->get() == on);
}

void TestFixture::check_adcs_dcdc_on(bool on) const
{
    TEST_ASSERT(adcs_dcdc_fp->get() == on);
}

// Ensures that no state except the given state can be achieved.
void TestFixture::assert_ground_uncommandability(adcs_state_t exception_state)
{
    for (adcs_state_t state_it : adcs_states)
    {
        if (state_it == exception_state)
            continue;
        set(state_it);
        step();
        TEST_ASSERT_NOT_EQUAL(static_cast<unsigned char>(state_it), adcs_state_fp->get());
    }
}

void TestFixture::assert_ground_uncommandability(prop_state_t exception_state)
{
    for (prop_state_t state_it : prop_states)
    {
        if (state_it == exception_state)
            continue;
        set(state_it);
        step();
        TEST_ASSERT_NOT_EQUAL(static_cast<unsigned int>(state_it), prop_state_fp->get());
    }
}

// Step forward the state machine by 1 control cycle.
void TestFixture::step()
{
    mission_manager->execute();
    mission_manager->control_cycle_count++;
}

void TestFixture::set_ccno(unsigned int ccno)
{
    MissionManager::control_cycle_count = ccno;
}

// Set the distance between the two satellites.
void TestFixture::set_sat_distance(double dist)
{
    rel_orbit_state_fp->set(static_cast<unsigned char>(rel_orbit_state_t::propagating));
    rel_orbit_rel_pos_fp->set({dist, 0.0, 0.0});
}

// Set the # of control cycles that comms has not been established
// with the ground.
void TestFixture::set_comms_blackout_period(int ccno)
{
    const unsigned int control_cycle_count = mission_manager->control_cycle_count;
    TEST_ASSERT_GREATER_OR_EQUAL(control_cycle_count, ccno);
    last_checkin_cycle_fp->set(control_cycle_count - ccno);
}

// Set the angular rate of the spacecraft.
void TestFixture::set_ang_rate(float rate)
{
    attitude_estimator_valid_fp->set(true);
    attitude_estimator_L_body_fp->set({gnc::constant::J_sat(0, 0) * rate, 0.0f, 0.0f});
}

// Get the bootcount
unsigned int TestFixture::get_bootcount()
{
    return mission_manager->bootcount_fp->get();
}


adcs_state_t TestFixture::adcs_states[9] = {adcs_state_t::detumble, adcs_state_t::limited, adcs_state_t::manual,
                                            adcs_state_t::point_docking, adcs_state_t::point_manual, adcs_state_t::point_standby,
                                            adcs_state_t::startup, adcs_state_t::zero_L, adcs_state_t::zero_torque};

prop_state_t TestFixture::prop_states[6] = {prop_state_t::disabled, prop_state_t::idle,
                                            prop_state_t::await_firing,
                                            prop_state_t::pressurizing,
                                            prop_state_t::firing,
                                            prop_state_t::venting};
