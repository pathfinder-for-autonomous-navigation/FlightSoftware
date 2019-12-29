#include <unity.h>
#include "test_fixture.hpp"

void test_valid_initialization() {
    TestFixture tf;
}

void test_dispatch_startup() {
    TestFixture tf;

    // Startup should be the default initial state of the mission manager
    tf.check(mission_state_t::startup);

    // For 10 executions, the mission manager should remain in the startup state
    for(int i = 0; i < 10; i++) {
        tf.step();
        tf.check(mission_state_t::startup);
    }

    // TODO add hardware fault test.

    // On the 11th execution, if there's no hardware fault, the mission manager
    // should transition to the detumble state.
    tf.step();
    tf.check(mission_state_t::detumble);
}

void test_dispatch_detumble() {
    TestFixture tf(mission_state_t::detumble);
    tf.adcs_min_stable_ang_rate_fp->set(5);

    // Stays in detumble mode if satellite is tumbling
    tf.set_ang_rate(6);
    tf.step();
    tf.check(sat_designation_t::undecided);
    tf.check(mission_state_t::detumble);
    tf.check(adcs_state_t::detumble);
    tf.check(prop_mode_t::disabled);
    tf.check(radio_mode_t::active);

    // If satellite is no longer tumbling, spacecraft exits detumble mode
    // and starts pointing in the expected direction.
    tf.set_ang_rate(4);
    tf.step();
    tf.check(adcs_state_t::point_standby);
    tf.check(mission_state_t::standby);
}

void test_dispatch_initialization_hold() {
    TestFixture tf(mission_state_t::initialization_hold);
    tf.step();
    tf.check(sat_designation_t::undecided);
    tf.check(mission_state_t::initialization_hold);
    tf.check(adcs_state_t::detumble);
    tf.check(prop_mode_t::disabled);
    tf.check(radio_mode_t::active);
}

void test_dispatch_rendezvous_state(mission_state_t mission_state,
    sat_designation_t sat_designation, adcs_state_t adcs_state, 
    prop_mode_t prop_mode, double trigger_distance,
    mission_state_t next_state)
{
    /** Test initialization **/
    {
        TestFixture tf(mission_state);
        tf.step();
        tf.check(sat_designation);
        tf.check(mission_state);
        tf.check(adcs_state);
        tf.check(prop_mode);
        tf.check(radio_mode_t::active);
    }

    /**
     * Test ground uncommandability of ADCS and propulsion state.
     */
    {
        TestFixture tf(mission_state);
        tf.assert_ground_uncommandability(adcs_state);
        tf.assert_ground_uncommandability(prop_mode);
    }

    /** If distance is less than the trigger distance,
        there should be a state transition to the next mission state. **/
    {
        TestFixture tf(mission_state);
        tf.set_sat_distance(trigger_distance - 0.01);
        tf.step();
        tf.check(next_state);
    }

    /** If comms hasn't been available for too long, there should
        be a state transition to standby.  **/
    {
        TestFixture tf(mission_state);
        tf.set_comms_blackout_period(MissionManager::max_radio_silence_duration + 1);
        tf.step();
        tf.check(adcs_state_t::point_standby);
        tf.check(mission_state_t::standby);
        tf.check(sat_designation_t::undecided);
    }
}

void test_dispatch_follower() {
    test_dispatch_rendezvous_state(mission_state_t::follower, sat_designation_t::follower,
        adcs_state_t::point_standby, prop_mode_t::active, MissionManager::close_approach_trigger_dist,
        mission_state_t::follower_close_approach);
}

void test_dispatch_leader() {
    test_dispatch_rendezvous_state(mission_state_t::leader, sat_designation_t::leader,
        adcs_state_t::point_standby, prop_mode_t::disabled, MissionManager::close_approach_trigger_dist,
        mission_state_t::leader_close_approach);
}

void test_dispatch_follower_close_approach() {
    test_dispatch_rendezvous_state(mission_state_t::follower_close_approach,
        sat_designation_t::follower, adcs_state_t::point_docking, prop_mode_t::active,
        MissionManager::docking_trigger_dist, mission_state_t::docking);
}

void test_dispatch_leader_close_approach() {
    test_dispatch_rendezvous_state(mission_state_t::leader_close_approach,
        sat_designation_t::leader, adcs_state_t::point_docking, prop_mode_t::active,
        MissionManager::docking_trigger_dist, mission_state_t::docking);
}

void test_dispatch_standby() {
    // Initialization test
    {
        TestFixture tf(mission_state_t::standby);
        tf.step();
        tf.check(mission_state_t::standby);
        tf.check(prop_mode_t::active);
        tf.check(radio_mode_t::active);
    }
    
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
    }

    // Standby -> leader transition test
    {
        TestFixture tf(mission_state_t::standby);
        tf.set(sat_designation_t::leader);
        tf.step();
        tf.check(mission_state_t::leader);
    }

    // Test that the ADCS state is fully ground-commandable, but that
    // propulsion mode is not. (Propulsion mode should always be active.)
    {
        TestFixture tf(mission_state_t::standby);

        for(adcs_state_t state : TestFixture::adcs_states) {
            tf.set(state);
            tf.step();
            TEST_ASSERT_EQUAL(static_cast<unsigned int>(state), tf.adcs_state_fp->get());
        }

        tf.assert_ground_uncommandability(prop_mode_t::active);
    }
}

void test_dispatch_docking() {
    TestFixture tf(mission_state_t::docking);
    tf.step();
    tf.check(mission_state_t::docking);
    tf.check(adcs_state_t::zero_torque);
    tf.check(prop_mode_t::disabled);
    tf.check(radio_mode_t::active);
    tf.assert_ground_uncommandability(adcs_state_t::zero_torque);
    tf.assert_ground_uncommandability(prop_mode_t::disabled);

    TEST_ASSERT(tf.docking_config_cmd_fp->get());

    // Pressing of the docking switch should cause state transition
    tf.docked_fp->set(true);
    tf.step();
    tf.check(mission_state_t::docked);
}

void test_dispatch_docked() {
    TestFixture tf(mission_state_t::docked);
    tf.step();
    tf.check(mission_state_t::docked);
    tf.check(adcs_state_t::zero_torque);
    tf.check(prop_mode_t::disabled);
    tf.check(radio_mode_t::active);
    tf.check(sat_designation_t::undecided);
    tf.assert_ground_uncommandability(adcs_state_t::zero_torque);
    tf.assert_ground_uncommandability(prop_mode_t::disabled);
}

void test_dispatch_paired() {
    TestFixture tf(mission_state_t::paired);
    tf.step();
    tf.check(adcs_state_t::set_paired_gains);
    tf.check(mission_state_t::standby);
    tf.check(sat_designation_t::undecided);
}

void test_dispatch_spacejunk() {
    TestFixture tf(mission_state_t::spacejunk);
    tf.step();
    tf.check(mission_state_t::spacejunk);
    tf.check(adcs_state_t::zero_L);
    tf.check(prop_mode_t::disabled);
    tf.check(radio_mode_t::active);
    tf.check(sat_designation_t::undecided);
    tf.assert_ground_uncommandability(adcs_state_t::zero_L);
    tf.assert_ground_uncommandability(prop_mode_t::disabled);
}

void test_dispatch_safehold() {
    TestFixture tf(mission_state_t::safehold);
    tf.step();
    tf.check(mission_state_t::safehold);
    tf.check(adcs_state_t::limited);
    tf.check(prop_mode_t::disabled);
    tf.check(radio_mode_t::active);
    tf.assert_ground_uncommandability(adcs_state_t::limited);
    tf.assert_ground_uncommandability(prop_mode_t::disabled);
}

void test_dispatch_manual() {
    TestFixture tf(mission_state_t::manual);
    tf.step();
    tf.check(mission_state_t::manual);
}

void test_dispatch_undefined() {
    TestFixture tf;
    tf.mission_state_fp->set(14); // Undefined
    tf.step();
    tf.check(mission_state_t::safehold);
}

int test_mission_manager() {
    UNITY_BEGIN();
    RUN_TEST(test_valid_initialization);
    RUN_TEST(test_dispatch_startup);
    RUN_TEST(test_dispatch_detumble);
    RUN_TEST(test_dispatch_initialization_hold);
    RUN_TEST(test_dispatch_follower);
    RUN_TEST(test_dispatch_follower_close_approach);
    RUN_TEST(test_dispatch_standby);
    RUN_TEST(test_dispatch_leader);
    RUN_TEST(test_dispatch_leader_close_approach);
    RUN_TEST(test_dispatch_docking);
    RUN_TEST(test_dispatch_docked);
    RUN_TEST(test_dispatch_paired);
    RUN_TEST(test_dispatch_spacejunk);
    RUN_TEST(test_dispatch_safehold);
    RUN_TEST(test_dispatch_manual);
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
