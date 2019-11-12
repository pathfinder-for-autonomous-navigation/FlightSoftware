#include "../StateFieldRegistryMock.hpp"
#include <StateField.hpp>
#include <Serializer.hpp>
#include "../src/FCCode/MissionManager.hpp"
#include "../src/FCCode/mission_mode_t.enum"

#include <unity.h>

class TestFixture {
  public:
    StateFieldRegistryMock registry;
    // Input state fields to mission manager
    std::shared_ptr<WritableStateField<unsigned int>> cycle_no_fp;
    std::shared_ptr<WritableStateField<unsigned int>> adcs_mode_fp;
    std::shared_ptr<WritableStateField<f_quat_t>> adcs_cmd_attitude_fp;
    std::shared_ptr<ReadableStateField<float>> adcs_ang_rate_fp;
    std::shared_ptr<WritableStateField<float>> adcs_min_stable_ang_rate_fp;

    std::unique_ptr<MissionManager> mission_manager;
    // Output state fields from mission manager
    std::shared_ptr<WritableStateField<unsigned int>> mission_mode_fp;

    TestFixture() : registry() {
        cycle_no_fp = registry.create_writable_field<unsigned int>("pan.cycle_no", 0, 4294967295, 32);
        adcs_mode_fp = registry.create_writable_field<unsigned int>("adcs.mode", 0, 10, 4) ;
        adcs_cmd_attitude_fp = registry.create_writable_field<f_quat_t>("adcs.cmd_attitude");
        adcs_ang_rate_fp = registry.create_readable_field<float>("adcs.ang_rate", 0, 10, 4);
        adcs_min_stable_ang_rate_fp = registry.create_writable_field<float>("adcs.min_stable_ang_rate", 0, 10, 4);
        mission_manager = std::make_unique<MissionManager>(registry, 0);

        mission_mode_fp = std::static_pointer_cast<WritableStateField<unsigned int>>(registry.find_writable_field("pan.mode"));
    }
};

void test_valid_initialization() {
    TestFixture tf;
}

void test_dispatch_detumble() {
    TestFixture tf;

    // Stays in detumble mode if satellite is tumbling
    tf.mission_mode_fp->set(static_cast<unsigned int>(mission_mode_t::detumble));
    tf.adcs_ang_rate_fp->set(6);
    tf.adcs_min_stable_ang_rate_fp->set(5);
    tf.mission_manager->execute();
    TEST_ASSERT_EQUAL(static_cast<unsigned int>(mission_mode_t::detumble), tf.mission_mode_fp->get());
    TEST_ASSERT_EQUAL(static_cast<unsigned int>(adcs_mode_t::detumble), tf.adcs_mode_fp->get());

    // If satellite is no longer tumbling, spacecraft exits detumble mode and starts pointing in 
    // the expected direction.
    tf.adcs_ang_rate_fp->set(4);
    tf.mission_manager->execute();
    TEST_ASSERT_EQUAL(static_cast<unsigned int>(mission_mode_t::standby), tf.mission_mode_fp->get());
    TEST_ASSERT_EQUAL(static_cast<unsigned int>(adcs_mode_t::point_standby), tf.adcs_mode_fp->get());
    f_quat_t expected_cmd_attitude = {2,2,2,2};
    f_quat_t actual_cmd_attitude = tf.adcs_cmd_attitude_fp->get();
    for(size_t i = 0; i < 4; i++) TEST_ASSERT_DOUBLE_WITHIN(0.01, expected_cmd_attitude[i], actual_cmd_attitude[i]);
}

int test_mission_manager() {
    UNITY_BEGIN();
    RUN_TEST(test_valid_initialization);
    RUN_TEST(test_dispatch_detumble);
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
