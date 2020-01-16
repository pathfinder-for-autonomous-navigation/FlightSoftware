#include "test_fixture.hpp"
#include <unity.h>
#include <limits>

TestFixture::TestFixture(mission_state_t initial_state) : registry()
{
    adcs_ang_momentum_fp = registry.create_internal_field<lin::Vector3f>(
                                "attitude_estimator.h_body");
    adcs_paired_fp = registry.create_writable_field<bool>("adcs.paired");

    radio_mode_fp = registry.create_internal_field<unsigned char>("radio.mode");
    last_checkin_cycle_fp = registry.create_internal_field<unsigned int>(
                                "radio.last_comms_ccno");

    prop_mode_fp = registry.create_readable_field<unsigned char>("prop.mode", 2);

    piksi_mode_fp = registry.create_readable_field<unsigned char>("piksi.state", 4);
    propagated_baseline_pos_fp = registry.create_readable_vector_field<double>(
                                    "orbit.baseline_pos", 0, 100000, 100);

    docked_fp = registry.create_readable_field<bool>("docksys.docked");

    // Initialize these variables
    const float nan_f = std::numeric_limits<float>::quiet_NaN();
    const double nan_d = std::numeric_limits<double>::quiet_NaN();
    adcs_ang_momentum_fp->set({nan_f,nan_f,nan_f});
    radio_mode_fp->set(static_cast<unsigned int>(radio_mode_t::disabled));
    last_checkin_cycle_fp->set(0);
    prop_mode_fp->set(static_cast<unsigned int>(prop_mode_t::disabled));
    propagated_baseline_pos_fp->set({nan_d,nan_d,nan_d});
    docked_fp->set(false);

    mission_manager = std::make_unique<MissionManager>(registry, 0);

    // Check that mission manager creates its expected fields
    adcs_state_fp = registry.find_writable_field_t<unsigned char>("adcs.state");
    docking_config_cmd_fp = registry.find_writable_field_t<bool>("docksys.config_cmd");
    mission_state_fp = registry.find_writable_field_t<unsigned char>("pan.state");
    is_deployed_fp = registry.find_readable_field_t<bool>("pan.deployed");
    deployment_wait_elapsed_fp = registry.find_readable_field_t<unsigned int>(
                                    "pan.deployment.elapsed");
    sat_designation_fp = registry.find_writable_field_t<unsigned char>("pan.sat_designation");

    // Set initial state.
    mission_state_fp->set(static_cast<unsigned int>(initial_state));
}

// Set and assert functions for various mission states.

void TestFixture::set(mission_state_t state) {
    mission_state_fp->set(static_cast<unsigned int>(state));
}

void TestFixture::set(adcs_state_t state) {
    adcs_state_fp->set(static_cast<unsigned int>(state));
}

void TestFixture::set(prop_mode_t mode) {
    prop_mode_fp->set(static_cast<unsigned int>(mode));
}

void TestFixture::set(radio_mode_t mode) {
    radio_mode_fp->set(static_cast<unsigned int>(mode));
}

void TestFixture::set(sat_designation_t designation) {
    sat_designation_fp->set(static_cast<unsigned int>(designation));
}

void TestFixture::check(mission_state_t state) const {
    TEST_ASSERT_EQUAL_MESSAGE(static_cast<unsigned int>(state), mission_state_fp->get(),
        "For mission state.");
}

void TestFixture::check(adcs_state_t state) const {
    TEST_ASSERT_EQUAL_MESSAGE(static_cast<unsigned int>(state), adcs_state_fp->get(),
        "For ADCS state.");
}

void TestFixture::check(prop_mode_t mode) const {
    TEST_ASSERT_EQUAL_MESSAGE(static_cast<unsigned int>(mode), prop_mode_fp->get(),
        "For propulsion mode.");
}

void TestFixture::check(radio_mode_t mode) const {
    TEST_ASSERT_EQUAL_MESSAGE(static_cast<unsigned int>(mode), radio_mode_fp->get(),
        "For radio mode.");
}

void TestFixture::check(sat_designation_t designation) const {
    TEST_ASSERT_EQUAL_MESSAGE(static_cast<unsigned int>(designation), sat_designation_fp->get(),
        "For satellite designation.");
}

// Ensures that no state except the given state can be achieved.
void TestFixture::assert_ground_uncommandability(adcs_state_t exception_state) {
    for(adcs_state_t state_it : adcs_states) {
        if (state_it == exception_state) continue;
        set(state_it);
        step();
        TEST_ASSERT_NOT_EQUAL(static_cast<unsigned int>(state_it), adcs_state_fp->get());
    }
}

void TestFixture::assert_ground_uncommandability(prop_mode_t exception_mode) {
    for(prop_mode_t mode_it : prop_modes) {
        if (mode_it == exception_mode) continue;
        set(mode_it);
        step();
        TEST_ASSERT_NOT_EQUAL(static_cast<unsigned int>(mode_it), prop_mode_fp->get());
    }
}

// Step forward the state machine by 1 control cycle.
void TestFixture::step() { mission_manager->execute(); }

void TestFixture::set_ccno(unsigned int ccno) {
    mission_manager->control_cycle_count = ccno;
}

// Create a hardware fault that necessitates a transition to safe hold or initialization hold.
void TestFixture::set_hardware_fault(bool faulted) {
    // TODO
}

// Set the distance between the two satellites.
void TestFixture::set_sat_distance(double dist) { propagated_baseline_pos_fp->set({dist, 0, 0}); }

// Set the # of control cycles that comms has not been established
// with the ground.
void TestFixture::set_comms_blackout_period(int ccno) {
    const unsigned int control_cycle_count = mission_manager->control_cycle_count;
    TEST_ASSERT_GREATER_OR_EQUAL(control_cycle_count, ccno);
    last_checkin_cycle_fp->set(control_cycle_count - ccno);
}

// Set the angular rate of the spacecraft.
void TestFixture::set_ang_rate(float rate) {
    adcs_ang_momentum_fp->set({rate, 0, 0}); // TODO will need to change this once the inertia tensor
                                             // is added to GNC constants.
}

adcs_state_t TestFixture::adcs_states[8] = {adcs_state_t::detumble, adcs_state_t::limited,
        adcs_state_t::point_docking, adcs_state_t::point_manual, adcs_state_t::point_standby,
        adcs_state_t::startup, adcs_state_t::zero_L, adcs_state_t::zero_torque};

prop_mode_t TestFixture::prop_modes[2] = {prop_mode_t::disabled, prop_mode_t::active};
