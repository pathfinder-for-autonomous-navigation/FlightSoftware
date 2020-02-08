#include <unity.h>
#include "test_fixture.hpp"
#include <fsw/FCCode/constants.hpp>
#include <adcs/constants.hpp>

void test_valid_initialization() {
    TestFixture tf;

    // Test initialized values
    TEST_ASSERT_EQUAL(0.2, tf.detumble_safety_factor_fp->get());
    TEST_ASSERT_EQUAL(100, tf.close_approach_trigger_dist_fp->get());
    TEST_ASSERT_EQUAL(0.4, tf.docking_trigger_dist_fp->get());
    TEST_ASSERT_EQUAL(PAN::one_day_ccno,
        tf.max_radio_silence_duration_fp->get());
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
                                * tf.detumble_safety_factor_fp->get();
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
        tf.check(adcs_state_t::point_standby);
    }

    // Standby -> leader transition test
    {
        TestFixture tf(mission_state_t::standby);
        tf.set(sat_designation_t::leader);
        tf.step();
        TEST_ASSERT_FALSE(tf.adcs_paired_fp->get());
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

#include <iostream>

void test_power_prop_adcs_faults_responsive() {
    // Any single one of the power and ADCS fault flags
    // should cause a transition to safehold if we're
    // in one of the fault-responsive states (see MissionManager.hpp).
    //
    // Prop pressurization fault should cause a transition to standby.
    //
    for(mission_state_t state : MissionManager::fault_responsive_states) {
        std::array<TestFixture, 5> tf_v {
            TestFixture(state), TestFixture(state), TestFixture(state),
            TestFixture(state), TestFixture(state)
        };
        tf_v[0].low_batt_fault_f.override();
        tf_v[1].wheel1_adc_fault_f.override();
        tf_v[2].wheel2_adc_fault_f.override();
        tf_v[3].wheel3_adc_fault_f.override();
        tf_v[4].wheel_pot_fault_f.override();
        for(TestFixture& tf : tf_v) {
            tf.step();
            tf.check(mission_state_t::safehold);
        }
    }
    for(mission_state_t state : MissionManager::fault_responsive_states) {
        TestFixture tf(state);
        tf.failed_pressurize_f.override();
        tf.step();
        tf.check(mission_state_t::standby);
    }

    // If an ADCS/power fault happens and a prop fault also happens, the ADCS fault
    // takes precedence in sending the satellite to standby.
    //
    // ADCS fault takes precedence over prop fault
    {
        TestFixture tf(mission_state_t::standby);
        tf.wheel1_adc_fault_f.override();
        tf.failed_pressurize_f.override();
        tf.step();
        tf.check(mission_state_t::safehold);
    }
    // Power fault takes precedence over prop fault
    {
        TestFixture tf(mission_state_t::standby);
        tf.low_batt_fault_f.override();
        tf.failed_pressurize_f.override();
        tf.step();
        tf.check(mission_state_t::safehold);
    }
}

void test_power_prop_adcs_faults_nonresponsive() {
    // The fault flags don't cause safehold if the mission
    // is currently in one of the fault-nonresponsive states
    // See MissionManager.hpp for a definition of these states.

    for(mission_state_t state : MissionManager::fault_nonresponsive_states) {
        std::array<TestFixture, 5> tf_v {
            TestFixture(state), TestFixture(state), TestFixture(state),
            TestFixture(state), TestFixture(state)
        };
        tf_v[0].low_batt_fault_f.override();
        tf_v[1].wheel1_adc_fault_f.override();
        tf_v[2].wheel2_adc_fault_f.override();
        tf_v[3].wheel3_adc_fault_f.override();
        tf_v[4].wheel_pot_fault_f.override();
        for(TestFixture& tf : tf_v) {
            tf.step();
            tf.check(state);
        }
    }
    for(mission_state_t state : MissionManager::fault_nonresponsive_states) {
        TestFixture tf(state);
        tf.failed_pressurize_f.override();
        tf.step();
        tf.check(state);
    }
}

void test_adcs_faults_inithold() {
    // Any ADCS fault should cause a transition to initialization
    // hold if the satellite is in startup past its
    // deployment wait period.
    {
        std::array<TestFixture, 4> tf_v;
        tf_v[0].wheel1_adc_fault_f.override();
        tf_v[1].wheel2_adc_fault_f.override();
        tf_v[2].wheel3_adc_fault_f.override();
        tf_v[3].wheel_pot_fault_f.override();
        for(TestFixture& tf : tf_v) {
            tf.deployment_wait_elapsed_fp->set(MissionManager::deployment_wait);
            tf.step();
            tf.check(mission_state_t::initialization_hold);
        }
    }
}

int test_mission_manager() {
    UNITY_BEGIN();
    RUN_TEST(test_valid_initialization);
    RUN_TEST(test_dispatch_startup);
    RUN_TEST(test_dispatch_detumble);
    RUN_TEST(test_dispatch_empty_states);
    RUN_TEST(test_dispatch_standby);
    RUN_TEST(test_rendezvous_states);
    RUN_TEST(test_dispatch_docking);
    RUN_TEST(test_dispatch_safehold);
    RUN_TEST(test_dispatch_undefined);

    RUN_TEST(test_power_prop_adcs_faults_responsive);
    RUN_TEST(test_power_prop_adcs_faults_nonresponsive);
    RUN_TEST(test_adcs_faults_inithold);

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
