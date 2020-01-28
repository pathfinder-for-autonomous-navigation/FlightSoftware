#include <unity.h>
#include "test_fixture.hpp"

#include <adcs/constants.hpp>

void test_valid_initialization() {
    TestFixture tf;

    // Test initialized values
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

    const float threshold = adcs::rwa::max_speed_read * adcs::rwa::moment_of_inertia
                                * MissionManager::detumble_safety_factor;
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
        TEST_ASSERT_FALSE(tf.adcs_paired_fp->get());
        tf.check(mission_state_t::follower);
        tf.check(sat_designation_t::follower);
        tf.check(adcs_state_t::point_docking);
    }

    // Standby -> leader transition test
    {
        TestFixture tf(mission_state_t::standby);
        tf.set(sat_designation_t::leader);
        tf.step();
        TEST_ASSERT_FALSE(tf.adcs_paired_fp->get());
        tf.check(mission_state_t::leader);
        tf.check(sat_designation_t::leader);
        tf.check(adcs_state_t::point_docking);
    }
}

void test_dispatch_rendezvous_state(mission_state_t mission_state)
{
    /** If distance is less than the trigger distance,
        there should be a state transition to the next mission state.
        This transition should happen irrespective of the comms timeout situation. **/
    {
        TestFixture tf(mission_state);
        tf.set_sat_distance(MissionManager::docking_trigger_dist - 0.01);
        tf.set_ccno(MissionManager::max_radio_silence_duration + 1);
        tf.set_comms_blackout_period(MissionManager::max_radio_silence_duration + 1);
        tf.step();
        tf.check(mission_state_t::docking);
        tf.check(prop_state_t::disabled);
        tf.check(adcs_state_t::zero_torque);
        tf.check(static_cast<sat_designation_t>(tf.sat_designation_fp->get()));

        // Docking motor command should be applied.
        TEST_ASSERT(tf.docking_config_cmd_fp->get());
    }

    /** If comms hasn't been available for too long, there should
        be a state transition to standby.  **/
    {
        TestFixture tf(mission_state);
        tf.set_ccno(MissionManager::max_radio_silence_duration + 1);
        tf.set_comms_blackout_period(MissionManager::max_radio_silence_duration + 1);
        tf.step();
        tf.check(prop_state_t::idle);
        tf.check(adcs_state_t::point_standby);
        tf.check(mission_state_t::standby);
        tf.check(sat_designation_t::undecided);
    }
}

void test_dispatch_follower() {
    test_dispatch_rendezvous_state(mission_state_t::follower);
}

void test_dispatch_leader() {
    test_dispatch_rendezvous_state(mission_state_t::leader);
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
}

void test_dispatch_safehold() {
    TestFixture tf(mission_state_t::safehold);
    // TODO
}

void test_dispatch_undefined() {
    TestFixture tf;
    tf.mission_state_fp->set(100); // Undefined
    tf.step();
    tf.check(mission_state_t::safehold);
}

int test_mission_manager() {
    UNITY_BEGIN();
    RUN_TEST(test_valid_initialization);
    RUN_TEST(test_dispatch_startup);
    RUN_TEST(test_dispatch_detumble);
    RUN_TEST(test_dispatch_empty_states);
    RUN_TEST(test_dispatch_follower);
    RUN_TEST(test_dispatch_standby);
    RUN_TEST(test_dispatch_leader);
    RUN_TEST(test_dispatch_docking);
    RUN_TEST(test_dispatch_safehold);
    RUN_TEST(test_dispatch_undefined);
    return UNITY_END();
}

#ifdef DESKTOP
int main() {
    return test_mission_manager();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_mission_manager();
}

void loop() {}
#endif
