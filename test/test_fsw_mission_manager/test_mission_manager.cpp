#include "../StateFieldRegistryMock.hpp"
#include <StateField.hpp>
#include <Serializer.hpp>
#include "../src/FCCode/MissionManager.hpp"
#include "../src/FCCode/mission_state_t.enum"
#include "../src/FCCode/adcs_state_t.enum"
#include "../src/FCCode/radio_mode_t.enum"
#include "../src/FCCode/prop_mode_t.enum"
#include "../src/FCCode/sat_designation_t.enum"

#include <unity.h>

class TestFixture {
  public:
    StateFieldRegistryMock registry;
    // Input state fields to mission manager
    std::shared_ptr<ReadableStateField<f_vector_t>> adcs_ang_vel_fp;
    std::shared_ptr<WritableStateField<float>> adcs_min_stable_ang_rate_fp;
    
    std::shared_ptr<InternalStateField<unsigned char>> radio_mode_fp;
    std::shared_ptr<InternalStateField<unsigned int>> last_checkin_cycle_fp;

    std::shared_ptr<ReadableStateField<unsigned char>> prop_mode_fp;

    std::shared_ptr<ReadableStateField<unsigned char>> piksi_mode_fp;
    std::shared_ptr<ReadableStateField<d_vector_t>> propagated_baseline_pos_fp;

    std::shared_ptr<ReadableStateField<bool>> docked_fp;

    std::unique_ptr<MissionManager> mission_manager;
    // Output state fields from mission manager
    WritableStateField<unsigned char>* adcs_state_fp;
    WritableStateField<bool>* docking_config_cmd_fp;
    WritableStateField<unsigned char>* mission_state_fp;
    ReadableStateField<bool>* is_deployed_fp;
    ReadableStateField<unsigned int>* deployment_wait_elapsed_fp;
    WritableStateField<unsigned char>* sat_designation_fp;

    TestFixture(mission_state_t initial_state = mission_state_t::startup)
        : registry()
    {
        adcs_ang_vel_fp = registry.create_readable_vector_field<float>(
                            "attitude_estimator.w_body", 0, 10, 100);
        adcs_min_stable_ang_rate_fp = registry.create_writable_field<float>(
                            "adcs.min_stable_ang_rate", 0, 10, 4);

        radio_mode_fp = registry.create_internal_field<unsigned char>("radio.mode");
        last_checkin_cycle_fp = registry.create_internal_field<unsigned int>(
                                    "radio.last_comms_ccno");

        prop_mode_fp = registry.create_readable_field<unsigned char>("prop.mode", 2);

        piksi_mode_fp = registry.create_readable_field<unsigned char>("piksi.state", 4);
        propagated_baseline_pos_fp = registry.create_readable_vector_field<double>(
                                        "orbit.baseline_pos", 0, 100000, 100);

        docked_fp = registry.create_readable_field<bool>("docksys.docked");

        // Initialize these variables
        adcs_ang_vel_fp->set({0,0,0});
        adcs_min_stable_ang_rate_fp->set(0);
        radio_mode_fp->set(static_cast<unsigned int>(radio_mode_t::disabled));
        last_checkin_cycle_fp->set(0);
        prop_mode_fp->set(static_cast<unsigned int>(prop_mode_t::disabled));
        propagated_baseline_pos_fp->set({0,0,0});
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

    void set(mission_state_t state) {
        mission_state_fp->set(static_cast<unsigned int>(state));
    }

    void set(adcs_state_t state) {
        adcs_state_fp->set(static_cast<unsigned int>(state));
    }

    void set(prop_mode_t mode) {
        prop_mode_fp->set(static_cast<unsigned int>(mode));
    }

    void set(radio_mode_t mode) {
        radio_mode_fp->set(static_cast<unsigned int>(mode));
    }

    void set(sat_designation_t designation) {
        sat_designation_fp->set(static_cast<unsigned int>(designation));
    }

    void check(mission_state_t state) const {
        TEST_ASSERT_EQUAL(static_cast<unsigned int>(state), mission_state_fp->get());
    }

    void check(adcs_state_t state) const {
        TEST_ASSERT_EQUAL(static_cast<unsigned int>(state), adcs_state_fp->get());
    }

    void check(prop_mode_t mode) const {
        TEST_ASSERT_EQUAL(static_cast<unsigned int>(mode), prop_mode_fp->get());
    }

    void check(radio_mode_t mode) const {
        TEST_ASSERT_EQUAL(static_cast<unsigned int>(mode), radio_mode_fp->get());
    }

    void check(sat_designation_t designation) const {
        TEST_ASSERT_EQUAL(static_cast<unsigned int>(designation), sat_designation_fp->get());
    }

    // Step forward the state machine by 1 control cycle.
    void step() { mission_manager->execute(); }

    //// Setter methods for setting test conditions.

    // Create a hardware fault that necessitates a transition to safe hold or initialization hold.
    void set_hardware_fault(bool faulted) {
        // TODO
    }

    // Set the distance between the two satellites.
    void set_sat_distance(double dist) { propagated_baseline_pos_fp->set({dist, 0, 0}); }

    // Set the # of control cycles that comms has not been established
    // with the ground.
    void set_comms_blackout_period(int ccno) {
        last_checkin_cycle_fp->set(mission_manager->control_cycle_count - ccno);
    }

    // Set the angular rate of the spacecraft.
    void set_ang_rate(float rate) {
        adcs_ang_vel_fp->set({rate, 0, 0});
    }
};

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

void test_rendezvous_state(mission_state_t mission_state, sat_designation_t sat_designation,
                           adcs_state_t adcs_state, double trigger_distance,
                           mission_state_t next_state)
{
    /** Test initialization **/
    {
        TestFixture tf(mission_state);
        tf.step();
        tf.check(sat_designation);
        tf.check(mission_state);
        tf.check(adcs_state);
        tf.check(prop_mode_t::active);
        tf.check(radio_mode_t::active);
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
    /** Ground should be unable to command the ADCS or propulsion modes. */
    /** TODO test. **/
}

void test_dispatch_follower() {
    test_rendezvous_state(mission_state_t::follower, sat_designation_t::follower,
        adcs_state_t::point_standby, MissionManager::close_approach_trigger_dist,
        mission_state_t::follower_close_approach);
}

void test_dispatch_follower_close_approach() {
    test_rendezvous_state(mission_state_t::follower_close_approach, sat_designation_t::follower,
        adcs_state_t::point_docking, MissionManager::docking_trigger_dist,
        mission_state_t::docking);
}

void test_dispatch_leader_close_approach() {
    test_rendezvous_state(mission_state_t::leader_close_approach, sat_designation_t::leader,
        adcs_state_t::point_docking, MissionManager::docking_trigger_dist,
        mission_state_t::docking);
}

#undef test_rendezvous_state

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
        tf.set_sat_distance(MissionManager::close_approach_trigger_dist - 0.01);
        tf.step();
        tf.check(mission_state_t::leader_close_approach);
    }
}

void test_dispatch_docking() {
    TestFixture tf(mission_state_t::docking);
    tf.step();
    tf.check(mission_state_t::docking);
    tf.check(adcs_state_t::zero_torque);
    tf.check(prop_mode_t::disabled);
    tf.check(radio_mode_t::active);

    TEST_ASSERT(tf.docking_config_cmd_fp->get());

    // Pressing of the docking switch should cause state transition
    tf.docked_fp->set(true);
    tf.step();
    tf.check(mission_state_t::docked);

    /** Ground should be unable to command the ADCS or propulsion modes. 
     * TODO test.
    */
}

void test_dispatch_docked() {
    TestFixture tf(mission_state_t::docked);
    tf.step();
    tf.check(mission_state_t::docked);
    tf.check(adcs_state_t::zero_torque);
    tf.check(prop_mode_t::disabled);
    tf.check(radio_mode_t::active);
    tf.check(sat_designation_t::undecided);

    /** Ground should be unable to command the ADCS or propulsion modes. 
     * TODO test.
    */
}

void test_dispatch_paired() {
    TestFixture tf(mission_state_t::paired);
    tf.step();
    tf.check(adcs_state_t::point_standby);
    tf.check(mission_state_t::standby);
    tf.check(sat_designation_t::undecided);

    /**
     * Ground should be unable to command the ADCS or propulsion modes. 
     * TODO test.
    */
}

void test_dispatch_spacejunk() {
    TestFixture tf(mission_state_t::spacejunk);
    tf.step();
    tf.check(mission_state_t::spacejunk);
    tf.check(adcs_state_t::zero_L);
    tf.check(prop_mode_t::disabled);
    tf.check(radio_mode_t::active);
    tf.check(sat_designation_t::undecided);

    /** 
     * Ground should be unable to command the ADCS or propulsion modes. 
     * TODO test.
    */
}

void test_dispatch_safehold() {
    TestFixture tf(mission_state_t::safehold);
    tf.step();
    tf.check(mission_state_t::safehold);
    tf.check(adcs_state_t::limited);
    tf.check(prop_mode_t::disabled);
    tf.check(radio_mode_t::active);

    /**
     * Ground should be unable to command the ADCS or propulsion modes. 
     * TODO test.
    */
}

void test_dispatch_manual() {
    TestFixture tf(mission_state_t::manual);
    tf.step();
    tf.check(mission_state_t::manual);
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
    RUN_TEST(test_dispatch_leader_close_approach);
    RUN_TEST(test_dispatch_docking);
    RUN_TEST(test_dispatch_docked);
    RUN_TEST(test_dispatch_paired);
    RUN_TEST(test_dispatch_spacejunk);
    RUN_TEST(test_dispatch_safehold);
    RUN_TEST(test_dispatch_manual);
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
