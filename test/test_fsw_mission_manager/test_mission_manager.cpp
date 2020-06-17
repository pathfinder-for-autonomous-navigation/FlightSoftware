#include "../custom_assertions.hpp"
#undef isnan
#undef isinf
#include "test_fixture.hpp"
#include <fsw/FCCode/constants.hpp>
#include <adcs/constants.hpp>
#include <gnc/constants.hpp>

namespace mission_manager_test {
void test_valid_initialization() {
    TestFixture tf;

    // Test initialized values
    TEST_ASSERT_EQUAL(0.2, tf.detumble_safety_factor_fp->get());
    TEST_ASSERT_EQUAL(100, tf.close_approach_trigger_dist_fp->get());
    TEST_ASSERT_EQUAL(0.4, tf.docking_trigger_dist_fp->get());
    TEST_ASSERT_EQUAL(PAN::one_day_ccno, tf.max_radio_silence_duration_fp->get());
    TEST_ASSERT_EQUAL(PAN::one_day_ccno, tf.docking_timeout_limit_fp->get());
    TEST_ASSERT(tf.docking_config_cmd_fp->get());
    TEST_ASSERT_FALSE(tf.is_deployed_fp->get());
    TEST_ASSERT_EQUAL(0, tf.deployment_wait_elapsed_fp->get());
    tf.check(sat_designation_t::undecided);
}

void test_dispatch_startup() {
    TestFixture tf;

    // Startup should be the default initial state of the mission manager
    tf.check(mission_state_t::startup);

    // For 100 executions, the mission manager should remain in the startup state
    for(int i = 0; i < 100; i++) {
        tf.step();
        tf.check(mission_state_t::startup);
    }

    // TODO add hardware fault test.

    // On the 101st execution, if there's no hardware fault, the mission manager
    // should transition to the detumble state.
    tf.step();
    tf.check(mission_state_t::detumble);
    tf.check_sph_dcdc_on(false);
    TEST_ASSERT(tf.is_deployed_fp->get());
}

void test_dispatch_empty_states() {
    // Initialization hold
    {
        TestFixture tf(mission_state_t::initialization_hold);
        tf.step();
        tf.check(mission_state_t::initialization_hold);
    }

    // Docked
    {
        TestFixture tf(mission_state_t::docked);
        tf.step();
        tf.check(mission_state_t::docked);
    }

    // Manual
    {
        TestFixture tf(mission_state_t::manual);
        tf.step();
        tf.check(mission_state_t::manual);
    }
}

void test_dispatch_detumble() {
    TestFixture tf(mission_state_t::detumble);
    tf.set(adcs_state_t::detumble);

    // Be aware, we assume here that J_sat is diagonal and that the set_ang_rate
    // function sets omega = rate x_hat.
    // This test may fail if gnc::constant::J_sat gets updated.
    const float threshold = adcs::rwa::max_speed_read * adcs::rwa::moment_of_inertia
                                * tf.detumble_safety_factor_fp->get() / gnc::constant::J_sat(0,0);
    const float delta = threshold * 0.01;

    // Stays in detumble mode if satellite is tumbling
    tf.set_ang_rate(threshold + delta);
    tf.step();
    tf.check(adcs_state_t::detumble);

    // If satellite is no longer tumbling, spacecraft exits detumble mode
    // and starts pointing in the expected direction.
    tf.set_ang_rate(threshold - delta);
    tf.step();
    tf.check(adcs_state_t::point_standby);
    tf.check(mission_state_t::standby);
    tf.check_sph_dcdc_on(true);
}

void test_dispatch_standby() {
    // The ground should be able to command the satellite into any
    // pointing mode from this point. We'll test point_standby,
    // point_manual, and detumble here.
    {
        TestFixture tf(mission_state_t::standby);
        tf.step();
        tf.check(adcs_state_t::startup);

        tf.set(adcs_state_t::point_standby); tf.step(); tf.check(adcs_state_t::point_standby);
        tf.set(adcs_state_t::point_manual);  tf.step(); tf.check(adcs_state_t::point_manual);
        tf.set(adcs_state_t::detumble);      tf.step(); tf.check(adcs_state_t::detumble);
    }

    // Standby -> follower transition test
    {
        TestFixture tf(mission_state_t::standby);
        tf.set(sat_designation_t::follower);
        tf.step();
        tf.check(mission_state_t::follower);
        tf.check(sat_designation_t::follower);
        tf.check(adcs_state_t::point_standby);
    }

    // Standby -> leader transition test
    {
        TestFixture tf(mission_state_t::standby);
        tf.set(sat_designation_t::leader);
        tf.step();
        tf.check(mission_state_t::leader);
        tf.check(sat_designation_t::leader);
        tf.check(adcs_state_t::point_standby);
    }
}

void test_dispatch_rendezvous_state(mission_state_t mission_state, double sat_distance)
{
    const bool in_close_approach = 
        (mission_state == mission_state_t::follower_close_approach) ||
        (mission_state == mission_state_t::leader_close_approach);

    /** If distance is less than the trigger distance,
        there should be a state transition to the next mission state.
        This transition should happen irrespective of the comms timeout situation. **/
    {
        TestFixture tf(mission_state);
        tf.set(prop_state_t::idle);
        tf.set_sat_distance(sat_distance);
        tf.set_ccno(tf.max_radio_silence_duration_fp->get() + 1);
        tf.set_comms_blackout_period(tf.max_radio_silence_duration_fp->get() + 1);
        tf.step();
        if (in_close_approach) {
            tf.check(mission_state_t::docking);
            tf.check(adcs_state_t::zero_torque);
            tf.check(prop_state_t::disabled);
            tf.check_sph_dcdc_on(true);
        }
        else {
            if (mission_state == mission_state_t::follower) {
                tf.check(mission_state_t::follower_close_approach);
                tf.check(prop_state_t::idle);
            }
            else {
                tf.check(mission_state_t::leader_close_approach);
                tf.check(prop_state_t::disabled);
            }
            tf.check_sph_dcdc_on(false);
            tf.check(adcs_state_t::point_docking);
        }
        tf.check(static_cast<sat_designation_t>(tf.sat_designation_fp->get()));

        // Docking motor command should be applied if we're in close approach
        if (in_close_approach) {
            TEST_ASSERT(tf.docking_config_cmd_fp->get());
        }
    }

    /** If comms hasn't been available for too long, there should be a state
     *  transition to standby.  **/
    {
        TestFixture tf(mission_state);
        tf.set_ccno(tf.max_radio_silence_duration_fp->get() + 1);
        tf.set_comms_blackout_period(tf.max_radio_silence_duration_fp->get() + 1);
        tf.step();
        tf.check_sph_dcdc_on(true);
        tf.check(prop_state_t::idle);
        tf.check(adcs_state_t::point_standby);
        tf.check(mission_state_t::standby);
        tf.check(sat_designation_t::undecided);
    }
}

void test_rendezvous_states() {
    TestFixture tf;
    const double close_approach_trigger = tf.close_approach_trigger_dist_fp->get() - 0.01;
    const double docking_trigger = tf.docking_trigger_dist_fp->get() - 0.01;

    test_dispatch_rendezvous_state(mission_state_t::follower, close_approach_trigger);
    test_dispatch_rendezvous_state(mission_state_t::leader, close_approach_trigger);
    test_dispatch_rendezvous_state(mission_state_t::follower_close_approach, docking_trigger);
    test_dispatch_rendezvous_state(mission_state_t::leader_close_approach, docking_trigger);
}

void test_dispatch_docking() {
    TestFixture tf(mission_state_t::docking);
    tf.step();

    // Docking motor command should be applied.
    TEST_ASSERT(tf.docking_config_cmd_fp->get());

    // Pressing of the docking switch should cause state transition
    tf.docked_fp->set(true);
    tf.step();
    tf.check(mission_state_t::docked);

    TestFixture tf2(mission_state_t::docking);
    tf2.step();

    // Check that the system is not docked
    TEST_ASSERT_FALSE(tf2.docked_fp->get());

    // Let a half day pass
    tf2.set_ccno(MissionManager::control_cycle_count+PAN::one_day_ccno/2 - 1);
    tf2.step();

    // Check that mission manager is still in a docking state
    tf2.check(mission_state_t::docking);

    // Let a nearly a full day pass
    tf2.set_ccno(MissionManager::control_cycle_count+PAN::one_day_ccno/2 - 1);
    tf2.step();

    // Check that mission manager is still in a docking state
    tf2.check(mission_state_t::docking);

    // Let a full day pass
    tf2.set_ccno(MissionManager::control_cycle_count+PAN::one_day_ccno/2);
    tf2.step();

    // Check that mission manager moves to standby
    tf2.check(mission_state_t::standby);

    // Even if a significant amount of time passes, mission managaer should still move to 
    // docked when the switch is pressed
    TestFixture tf3(mission_state_t::docking);
    tf3.step();

    tf3.docked_fp->set(true);
    tf3.set_ccno(MissionManager::control_cycle_count+5*PAN::one_day_ccno);
    tf3.step();

    tf3.check(mission_state_t::docked);
    tf3.check_sph_dcdc_on(false);
}

void test_dispatch_safehold() {
    // Test that a satellite reboot is correctly triggered.
    {
        TestFixture tf(mission_state_t::safehold);
        tf.check_sph_dcdc_on(false);

        // Below one day's worth of cycle counts, safe hold should
        // not trigger a satellite reboot.
        tf.set_ccno(PAN::one_day_ccno - 1);
        tf.step();
        TEST_ASSERT_FALSE(tf.reboot_fp->get());

        // Above a day's worth of cycle counts, safe hold should
        // trigger a satellite reboot.
        tf.step();
        TEST_ASSERT_TRUE(tf.reboot_fp->get());
    }

    // Test that moving in and out of safehold resets the 
    // cycle # at which safehold began. So entering safe hold
    // again means that the satellite waits a full 24 hours before
    // triggering a reboot.
    {
        TestFixture tf(mission_state_t::safehold);
        tf.set_ccno(PAN::one_day_ccno - 1);
        tf.step();
        TEST_ASSERT_FALSE(tf.reboot_fp->get());

        tf.set(mission_state_t::standby);
        tf.step();
        tf.set(mission_state_t::safehold);
        tf.step();
        TEST_ASSERT_FALSE(tf.reboot_fp->get());

        tf.set_ccno(2*PAN::one_day_ccno);
        tf.step();
        TEST_ASSERT_FALSE(tf.reboot_fp->get());
        tf.step();
        TEST_ASSERT_TRUE(tf.reboot_fp->get());
    }
}

void test_dispatch_undefined() {
    TestFixture tf;
    tf.mission_state_fp->set(100); // Undefined
    tf.step();
    tf.check(mission_state_t::safehold);
}
}

namespace fsw_test {
void test_mission_manager() {
    UNITY_BEGIN();
    RUN_TEST(mission_manager_test::test_valid_initialization);
    RUN_TEST(mission_manager_test::test_dispatch_startup);
    RUN_TEST(mission_manager_test::test_dispatch_detumble);
    RUN_TEST(mission_manager_test::test_dispatch_empty_states);
    RUN_TEST(mission_manager_test::test_dispatch_standby);
    RUN_TEST(mission_manager_test::test_rendezvous_states);
    RUN_TEST(mission_manager_test::test_dispatch_docking);
    RUN_TEST(mission_manager_test::test_dispatch_safehold);
    RUN_TEST(mission_manager_test::test_dispatch_undefined);
    // TODO add fault handling tests
    UNITY_END();
}
}

#ifndef COMBINE_TESTS
UNIT_TEST_RUNNER(fsw_test::test_mission_manager);
#endif
