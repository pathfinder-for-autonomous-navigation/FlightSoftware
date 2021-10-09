#include "../custom_assertions.hpp"
#undef isnan
#undef isinf
#include "test_fixture.hpp"
#include <fsw/FCCode/constants.hpp>
#include <adcs/constants.hpp>
#include <gnc/constants.hpp>

void test_valid_initialization() {
    TestFixture tf;

    // Test initialized values
    TEST_ASSERT_EQUAL(0.2, tf.detumble_safety_factor_fp->get());
    TEST_ASSERT_EQUAL(2000, tf.close_approach_trigger_dist_fp->get());
    TEST_ASSERT_EQUAL(0.4, tf.docking_trigger_dist_fp->get());
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

    // For the duration of the deployment hold, the mission manager should
    // remain in the startup state
    for(size_t i = 0; i < PAN::one_day_ccno / (24 * 2); i++) {
        tf.step();
        tf.check(mission_state_t::startup);
    }

    // TODO add hardware fault test.

    // On the 101st execution, if there's no hardware fault, the mission manager
    // should transition to the detumble state.
    tf.step();
    tf.check(mission_state_t::detumble);
    tf.check_sph_dcdc_on(true);
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
    // TODO
    tf.sph_dcdc_fp->set(true);

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
    tf.check_adcs_dcdc_on(false);
    
    // check that the ADCS DCDC is turned on pre-emptively but mission mode stays in detumble
    tf.step();
    tf.check_adcs_dcdc_on(true);
    tf.check(mission_state_t::detumble);

    // check that the second time around the mission mode does transition
    tf.step();
    tf.check(adcs_state_t::point_standby);
    tf.check(mission_state_t::standby);
    // TODO
    tf.check_sph_dcdc_on(true);
    tf.check_adcs_dcdc_on(true);
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
        tf.set(adcs_state_t::manual);  tf.step(); tf.check(adcs_state_t::manual);
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
        there should be a state transition to the next mission state. **/
    {
        TestFixture tf(mission_state);
        tf.set(prop_state_t::idle);
        tf.sph_dcdc_fp->set(true);
        tf.set_sat_distance(sat_distance);
        tf.step();
        if (in_close_approach) {
            tf.check(mission_state_t::docking);
            tf.check(adcs_state_t::zero_torque);
            tf.check(prop_state_t::idle);
            tf.check_sph_dcdc_on(true);
        }
        else {
            if (mission_state == mission_state_t::follower) {
                tf.check(mission_state_t::follower_close_approach);
                tf.check(prop_state_t::idle);
            }
            else {
                tf.check(mission_state_t::leader_close_approach);
                tf.check(prop_state_t::idle);
            }
            tf.check_sph_dcdc_on(true);
            tf.check(adcs_state_t::point_docking);
        }
        tf.check(static_cast<sat_designation_t>(tf.sat_designation_fp->get()));

        // Docking motor command should be applied if we're in close approach
        if (in_close_approach) {
            TEST_ASSERT(tf.docking_config_cmd_fp->get());
        }
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

    // Even if a significant amount of time passes, mission manager should still move to 
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
        tf.check_adcs_dcdc_on(false);

        // Below one day's worth of cycle counts, safe hold should
        // not trigger a satellite reboot.
        tf.set_ccno(PAN::one_day_ccno - 1);
        tf.step();
        TEST_ASSERT_FALSE(tf.reset_fp->get());

        // Above a day's worth of cycle counts, safe hold should
        // trigger a satellite reboot.
        tf.step();
        TEST_ASSERT_TRUE(tf.reset_fp->get());
    }

    // Test that moving in and out of safehold resets the 
    // cycle # at which safehold began. So entering safe hold
    // again means that the satellite waits a full 24 hours before
    // triggering a reboot.
    {
        TestFixture tf(mission_state_t::safehold);
        tf.set_ccno(PAN::one_day_ccno - 1);
        tf.step();
        TEST_ASSERT_FALSE(tf.reset_fp->get());

        tf.set(mission_state_t::standby);
        tf.step();
        tf.set(mission_state_t::safehold);
        tf.step();
        TEST_ASSERT_FALSE(tf.reset_fp->get());

        tf.set_ccno(2*PAN::one_day_ccno);
        tf.step();
        TEST_ASSERT_FALSE(tf.reset_fp->get());
        tf.step();
        TEST_ASSERT_TRUE(tf.reset_fp->get());
    }
}

void test_dispatch_undefined() {
    TestFixture tf;
    tf.mission_state_fp->set(100); // Undefined
    tf.step();
    tf.check(mission_state_t::safehold);
    tf.check_adcs_dcdc_on(false);
}

void test_fault_responses() {
    // No fault response recommendation shall be respected in startup
    // or manual.
    for(mission_state_t initial_state : {mission_state_t::startup,
                                         mission_state_t::manual})
    {
        TestFixture tf{initial_state};
        tf.set(fault_response_t::standby);
        tf.step();
        tf.check(initial_state);
    }
    for(mission_state_t initial_state : {mission_state_t::startup,
                                         mission_state_t::manual})
    {
        TestFixture tf{initial_state};
        tf.set(fault_response_t::safehold);
        tf.step();
        tf.check(initial_state);
    }

    // Fault response recommendations are redundant in safehold.
    {
        TestFixture tf{mission_state_t::safehold};
        tf.set(fault_response_t::standby);
        tf.step();
        tf.check(mission_state_t::safehold);
        tf.set(fault_response_t::safehold);
        tf.step();
        tf.check(mission_state_t::safehold);
    }

    // Only safehold fault recommendations shake the mission
    // manager out of initialization hold or detumble.
    for(mission_state_t initial_state : {mission_state_t::initialization_hold,
                                         mission_state_t::detumble})
    {
        TestFixture tf{initial_state};
        tf.set(fault_response_t::standby);
        tf.step();
        tf.check(initial_state);
        tf.set(fault_response_t::safehold);
        tf.step();
        tf.check(mission_state_t::safehold);
    }

    // All other states can experience transitions to safehold or standby
    // due to a fault response recommendation. 
    for(mission_state_t initial_state : {mission_state_t::standby,
                                         mission_state_t::follower,
                                         mission_state_t::leader,
                                         mission_state_t::follower_close_approach,
                                         mission_state_t::leader_close_approach,
                                         mission_state_t::docking,
                                         mission_state_t::docked})
    {
        TestFixture tf{initial_state};
        tf.adcs_dcdc_fp->set(true);
        tf.set(fault_response_t::standby);
        tf.step();
        tf.check(mission_state_t::standby);
        tf.check_adcs_dcdc_on(true);

        TestFixture tf2{initial_state};
        tf.adcs_dcdc_fp->set(true);
        tf2.set(fault_response_t::safehold);
        tf2.step();
        tf2.check(mission_state_t::safehold);
        tf2.check_adcs_dcdc_on(false);
    }
}

void test_bootcount(){
    // Test that the bootcount correctly updates and deployment is 
    // not delayed when bootcount >1

    TestFixture tf;

    // Test if the bootcount field exists in EEPROM
    // Can be found in telemetry file
    TEST_ASSERT_TRUE(tf.registry.find_eeprom_saved_field("pan.bootcount") != nullptr); 
        
    // Should initialize at 0
    TEST_ASSERT_EQUAL(0, tf.get_bootcount());  

    
    // Value increments by one when init() function is called
    tf.mission_manager->init();    
    TEST_ASSERT_EQUAL(1, tf.get_bootcount());

    // Further EEPROM bootcount testing is waived per PTest results in #813.        
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
    RUN_TEST(test_fault_responses);
    RUN_TEST(test_bootcount);
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
