#include "../StateFieldRegistryMock.hpp"
#include <StateField.hpp>
#include <Serializer.hpp>
#include "../src/FCCode/MissionManager.hpp"
#include "../src/FCCode/mission_state_t.enum"
#include "../src/FCCode/adcs_state_t.enum"

#include <unity.h>

#define set_mission_state(state) \
    tf.mission_state_fp->set(static_cast<unsigned int>(mission_state_t::state));
#define set_adcs_state(state) \
    tf.adcs_state_fp->set(static_cast<unsigned int>(adcs_state_t::state));
#define set_prop_mode(mode) \
    tf.prop_mode_fp->set(static_cast<unsigned int>(propulsion_mode_t::mode));
#define set_radio_mode(mode) \
    tf.radio_mode_fp->set(static_cast<unsigned int>(radio_mode_t::mode));
#define set_satellite_designation(designation) \
    tf.sat_designation_fp->set(static_cast<unsigned int>(satellite_designation_t::designation));

#define assert_mission_state(state) \
    TEST_ASSERT_EQUAL(static_cast<unsigned int>(mission_state_t::state), \
                      tf.mission_state_fp->get());
#define assert_adcs_state(state) \
    TEST_ASSERT_EQUAL(static_cast<unsigned int>(adcs_state_t::state), tf.adcs_state_fp->get());
#define assert_prop_mode(mode) \
    TEST_ASSERT_EQUAL(static_cast<unsigned int>(prop_mode_t::mode), tf.prop_mode_fp->get());
#define assert_radio_mode(mode) \
    TEST_ASSERT_EQUAL(static_cast<unsigned int>(radio_mode_t::mode), tf.radio_mode_fp->get());
#define assert_satellite_designation(designation) \
    TEST_ASSERT_EQUAL(static_cast<unsigned int>(satellite_designation_t::designation), \
                      tf.sat_designation_fp->get());


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
    WritableStateField<unsigned char>* sat_designation_fp;

    TestFixture(mission_state_t initial_state) : registry() {
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

        mission_manager = std::make_unique<MissionManager>(registry, 0);

        // Check that mission manager creates its expected fields
        adcs_state_fp = registry.find_writable_field_t<unsigned char>("adcs.state");
        docking_config_cmd_fp = registry.find_writable_field_t<bool>("docksys.config_cmd");
        mission_state_fp = registry.find_writable_field_t<unsigned char>("pan.state");
        is_deployed_fp = registry.find_readable_field_t<bool>("pan.deployed");
        deployment_wait_elapsed_fp = registry.find_readable_field_t<bool>(
                                        "pan.deployment.elapsed");
        sat_designation_fp = registry.find_writable_field_t<unsigned char>("pan.sat_designation");

        // Set initial state.
        mission_state_fp->set(static_cast<unsigned int>(initial_state));
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

    void set_comms_blackout_period(int ccno) {
        last_checkin_cycle_fp->set(mission_manager->control_cycle_count - ccno);
    }

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
    assert_mission_state(startup);

    // For less than 10 executions, the mission manager should remain in the startup state
    for(int i = 0; i < 9; i++) {
        tf.step();
        assert_mission_state(startup);
    }

    // TODO add hardware fault test.

    // On the 10th execution, if there's no hardware fault, the mission manager
    // should transition to the standby state.
    tf.step();
    assert_mission_state(standby);
}

void test_dispatch_detumble() {
    TestFixture tf(mission_state_t::detumble);
    tf.adcs_min_stable_ang_rate_fp->set(5);

    // Stays in detumble mode if satellite is tumbling
    tf.set_ang_rate(5);
    tf.step();
    assert_satellite_designation(undecided);
    assert_mission_state(detumble);
    assert_adcs_state(detumble);
    assert_prop_mode(disabled);
    assert_radio_mode(active);

    // If satellite is no longer tumbling, spacecraft exits detumble mode
    // and starts pointing in the expected direction.
    tf.set_ang_rate(4);
    tf.step();
    assert_mission_state(standby);
}

void test_dispatch_initialization_hold() {
    TestFixture tf(mission_state_t::initialization_hold);
    tf.step();
    assert_satellite_designation(undecided);
    assert_mission_state(initialization_hold);
    assert_adcs_state(detumble);
    assert_prop_mode(disabled);
    assert_radio_mode(active);
}

#define test_rendezvous_state(mission_state, satellite_designation, adcs_state, trigger_distance, next_state) { \
    // Test initialization
    { \
        TestFixture tf(mission_state_t::mission_state); \
        tf.step(); \
        assert_satellite_designation(satellite_designation); \
        assert_mission_state(mission_state); \
        assert_adcs_state(adcs_state); \
        assert_prop_mode(active); \
        assert_radio_mode(active); \
    } \

    // If distance is less than the trigger distance,
    // there should be a state transition to the next mission state.
    { \
        TestFixture tf(mission_state_t::mission_state); \
        tf.set_sat_distance(trigger_distance - 0.01); \
        tf.step(); \
        assert_mission_state(next_state); \
    } \

    // If comms hasn't been available for too long, there should
    // be a state transition to standby.
    { \
        TestFixture tf(mission_state_t::mission_state); \
        tf.set_comms_blackout_period(MissionManager::max_radio_silence_duration + 1); \
        tf.step(); \
        assert_mission_state(standby); \
        assert_satellite_designation(undecided); \
    } \
}

void test_dispatch_follower() {
    test_rendezvous_state(follower, follower, point_standby,
        MissionManager::close_approach_trigger_dist, follower_close_approach);
}

void test_dispatch_follower_close_approach() {
    test_rendezvous_state(follower_close_approach, follower, point_docking,
        MissionManager::docking_trigger_dist, docking);
}

void test_dispatch_leader_close_approach() {
    test_rendezvous_state(leader_close_approach, leader, point_docking,
        MissionManager::docking_trigger_dist, docking);
}

void test_dispatch_standby() {
    // Initialization test
    {
        TestFixture tf(mission_state_t::standby);
        tf.step();
        assert_mission_state(standby);
        assert_adcs_state(point_standby);
        assert_prop_mode(active);
        assert_radio_mode(active);
    }

    // Standby -> follower transition test
    {
        TestFixture tf(mission_state_t::standby);
        set_satellite_designation(follower);
        tf.step();
        assert_mission_state(follower);
    }

    // Standby -> leader transition test
    {
        TestFixture tf(mission_state_t::standby);
        set_satellite_designation(leader);
        // TODO set distance to other satellite
        tf.step();
        assert_mission_state(leader_close_approach);
    }
}

void test_dispatch_docking() {
    TestFixture tf(mission_state_t::docking);
    tf.step();
    assert_mission_state(docking);
    assert_adcs_state(zero_torque);
    assert_prop_mode(disabled);
    assert_radio_mode(active);

    TEST_ASSERT(tf.docking_config_cmd_fp->get());

    // Pressing of the docking switch should cause state transition
    tf.docked_fp->set(true);
    tf.step();
    assert_mission_state(docked);
}

void test_dispatch_docked() {
    TestFixture tf(mission_state_t::docked);
    tf.step();
    assert_mission_state(docked);
    assert_adcs_state(zero_torque);
    assert_prop_mode(disabled);
    assert_radio_mode(active);
    assert_satellite_designation(undecided);
}

void test_dispatch_paired() {
    TestFixture tf(mission_state_t::paired);
    tf.step();
    assert_mission_state(standby);
    assert_satellite_designation(undecided);
}

void test_dispatch_spacejunk() {
    TestFixture tf(mission_state_t::spacejunk);
    tf.step();
    assert_mission_state(spacejunk);
    assert_adcs_state(zero_L);
    assert_prop_mode(disabled);
    assert_radio_mode(active);
    assert_satellite_designation(undecided);
}

void test_dispatch_safehold() {
    TestFixture tf(mission_state_t::safehold);
    tf.step();
    assert_mission_state(safehold);
    assert_adcs_state(limited);
    assert_prop_mode(disabled);
    assert_radio_mode(active);
}

void test_dispatch_manual() {
    TestFixture tf(mission_state_t::manual);
    tf.step();
    assert_mission_state(manual);
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

#undef set_mission_state
#undef set_adcs_state
#undef set_prop_mode
#undef set_radio_mode
#undef set_satellite_designation

#undef assert_mission_state
#undef assert_adcs_state
#undef assert_prop_mode
#undef assert_radio_mode
#undef assert_satellite_designation
