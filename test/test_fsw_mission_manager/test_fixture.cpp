#include "test_fixture.hpp"
#include "../FaultHandlerMachineMock.hpp"
#include <unity.h>
#include <limits>

TestFixture::TestFixture(mission_state_t initial_state) : registry() {
    adcs_ang_momentum_fp = registry.create_internal_field<lin::Vector3f>(
                                "attitude_estimator.h_body");
    adcs_paired_fp = registry.create_writable_field<bool>("adcs.paired");

    radio_state_fp = registry.create_internal_field<unsigned char>("radio.state");
    last_checkin_cycle_fp = registry.create_internal_field<unsigned int>(
                                "radio.last_comms_ccno");

    prop_state_fp = registry.create_readable_field<unsigned char>("prop.state", 2);

    propagated_baseline_pos_fp = registry.create_readable_lin_vector_field<double>(
                                    "orbit.baseline_pos", 0, 100000, 100);

    reboot_fp = registry.create_writable_field<bool>("gomspace.gs_reboot_cmd");
    power_cycle_radio_fp = registry.create_writable_field<bool>("gomspace.power_cycle_output1_cmd");

    docked_fp = registry.create_readable_field<bool>("docksys.docked");

    low_batt_fault_fp=registry.create_fault("gomspace.low_batt", 1, TimedControlTaskBase::control_cycle_count);
    adcs_functional_fault_fp=registry.create_fault("adcs_monitor.functional_fault", 1, TimedControlTaskBase::control_cycle_count);
    wheel1_adc_fault_fp=registry.create_fault("adcs_monitor.wheel1_fault", 1, TimedControlTaskBase::control_cycle_count);
    wheel2_adc_fault_fp=registry.create_fault("adcs_monitor.wheel2_fault", 1, TimedControlTaskBase::control_cycle_count);
    wheel3_adc_fault_fp=registry.create_fault("adcs_monitor.wheel3_fault", 1, TimedControlTaskBase::control_cycle_count);
    wheel_pot_fault_fp=registry.create_fault("adcs_monitor.wheel_pot_fault", 1, TimedControlTaskBase::control_cycle_count);
    failed_pressurize_fp=registry.create_fault("prop.failed_pressurize", 1, TimedControlTaskBase::control_cycle_count);
    overpressured_fp=registry.create_fault("prop.overpressured", 1, TimedControlTaskBase::control_cycle_count);

    // Initialize these variables
    const float nan_f = std::numeric_limits<float>::quiet_NaN();
    const double nan_d = std::numeric_limits<double>::quiet_NaN();
    adcs_ang_momentum_fp->set({nan_f,nan_f,nan_f});
    radio_state_fp->set(static_cast<unsigned char>(radio_state_t::disabled));
    last_checkin_cycle_fp->set(0);
    prop_state_fp->set(static_cast<unsigned char>(prop_state_t::disabled));
    propagated_baseline_pos_fp->set({nan_d,nan_d,nan_d});
    reboot_fp->set(false);
    power_cycle_radio_fp->set(false);
    docked_fp->set(false);

    mission_manager = std::make_unique<MissionManager>(registry, 0);

    // Check that mission manager creates its expected fields
    detumble_safety_factor_fp = registry.find_writable_field_t<double>("detumble_safety_factor");
    close_approach_trigger_dist_fp = registry.find_writable_field_t<double>("trigger_dist.close_approach");
    docking_trigger_dist_fp = registry.find_writable_field_t<double>("trigger_dist.docking");
    max_radio_silence_duration_fp = registry.find_writable_field_t<unsigned int>("max_radio_silence");
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

void TestFixture::set(fault_response_t response) {
    static_cast<FaultHandlerMachineMock*>(
        mission_manager->main_fault_handler.get())->set(response);
}

void TestFixture::set(mission_state_t state) {
    mission_manager->set(state);
}

void TestFixture::set(adcs_state_t state) {
    adcs_state_fp->set(static_cast<unsigned char>(state));
}

void TestFixture::set(prop_state_t state) {
    prop_state_fp->set(static_cast<unsigned char>(state));
}

void TestFixture::set(radio_state_t state) {
    radio_state_fp->set(static_cast<unsigned char>(state));
}

void TestFixture::set(sat_designation_t designation) {
    sat_designation_fp->set(static_cast<unsigned char>(designation));
}

void TestFixture::check(mission_state_t state) const {
    TEST_ASSERT_EQUAL_MESSAGE(static_cast<unsigned char>(state), mission_state_fp->get(),
        "For mission state.");
}

void TestFixture::check(adcs_state_t state) const {
    TEST_ASSERT_EQUAL_MESSAGE(static_cast<unsigned char>(state), adcs_state_fp->get(),
        "For ADCS state.");
}

void TestFixture::check(prop_state_t state) const {
    TEST_ASSERT_EQUAL_MESSAGE(static_cast<unsigned char>(state), prop_state_fp->get(),
        "For propulsion state.");
}

void TestFixture::check(radio_state_t state) const {
    TEST_ASSERT_EQUAL_MESSAGE(static_cast<unsigned char>(state), radio_state_fp->get(),
        "For radio state.");
}

void TestFixture::check(sat_designation_t designation) const {
    TEST_ASSERT_EQUAL_MESSAGE(static_cast<unsigned char>(designation), sat_designation_fp->get(),
        "For satellite designation.");
}

// Ensures that no state except the given state can be achieved.
void TestFixture::assert_ground_uncommandability(adcs_state_t exception_state) {
    for(adcs_state_t state_it : adcs_states) {
        if (state_it == exception_state) continue;
        set(state_it);
        step();
        TEST_ASSERT_NOT_EQUAL(static_cast<unsigned char>(state_it), adcs_state_fp->get());
    }
}

void TestFixture::assert_ground_uncommandability(prop_state_t exception_state) {
    for(prop_state_t state_it : prop_states) {
        if (state_it == exception_state) continue;
        set(state_it);
        step();
        TEST_ASSERT_NOT_EQUAL(static_cast<unsigned char>(state_it), prop_state_fp->get());
    }
}

// Step forward the state machine by 1 control cycle.
void TestFixture::step() {
    mission_manager->execute();
    mission_manager->control_cycle_count++;
}

void TestFixture::set_ccno(unsigned int ccno) {
    mission_manager->control_cycle_count = ccno;
}

// Set the distance between the two satellites.
void TestFixture::set_sat_distance(double dist) {
    lin::Vector3d temp;
    temp(0) = dist; temp(1) = 0; temp(2) = 0;
    propagated_baseline_pos_fp->set(temp);
}

// Set the # of control cycles that comms has not been established
// with the ground.
void TestFixture::set_comms_blackout_period(int ccno) {
    const unsigned int control_cycle_count = mission_manager->control_cycle_count;
    TEST_ASSERT_GREATER_OR_EQUAL(control_cycle_count, ccno);
    last_checkin_cycle_fp->set(control_cycle_count - ccno);
}

// Set the angular rate of the spacecraft.
void TestFixture::set_ang_rate(float rate) {
    adcs_ang_momentum_fp->set({rate, 0.0f, 0.0f}); // TODO will need to change this once the inertia tensor
                                             // is added to GNC constants.
}

adcs_state_t TestFixture::adcs_states[8] = {adcs_state_t::detumble, adcs_state_t::limited,
        adcs_state_t::point_docking, adcs_state_t::point_manual, adcs_state_t::point_standby,
        adcs_state_t::startup, adcs_state_t::zero_L, adcs_state_t::zero_torque};

prop_state_t TestFixture::prop_states[6] = {prop_state_t::disabled, prop_state_t::idle,
    prop_state_t::await_firing,
    prop_state_t::pressurizing,
    prop_state_t::firing,
    prop_state_t::venting};
