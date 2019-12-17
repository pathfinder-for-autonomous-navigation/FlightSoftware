#include "../StateFieldRegistryMock.hpp"
#include "../../src/FCCode/EEPROMController.hpp"

#include <unity.h>

class TestFixture {
  public:
    StateFieldRegistryMock registry;

    std::shared_ptr<WritableStateField<unsigned char>> mission_mode_fp;
    std::shared_ptr<ReadableStateField<bool>> is_deployed_fp;
    std::shared_ptr<WritableStateField<unsigned char>> sat_designation_fp;
    std::shared_ptr<ReadableStateField<unsigned int>> control_cycle_count_fp;

    std::unique_ptr<EEPROMController> eeprom_controller;

    TestFixture() : registry() {
        mission_mode_fp = registry.create_writable_field<bool>("pan.mode");
        mission_mode_fp->set(1);

        is_deployed_fp = registry.create_readable_field<bool>("pan.deployed");
        is_deployed_fp->set(false);

        sat_designation_fp = registry.create_writable_field<bool>("pan.sat_designation");
        sat_designation_fp->set(3);

        control_cycle_count_fp = registry.create_readable_field<bool>("pan.cycle_no");
        control_cycle_count_fp->set(44);

        eeprom_controller = std::make_unique<EEPROMController>(registry, 0); 
    }
};

void test_task_initialization() {
    TestFixture tf;

    TEST_ASSERT_EQUAL(1, tf.mission_mode_fp->get());
    TEST_ASSERT_EQUAL(false, tf.is_deployed_fp->get());
    TEST_ASSERT_EQUAL(3, tf.sat_designation_fp->get());
    TEST_ASSERT_EQUAL(44, tf.control_cycle_count_fp->get());
}

void test_task_execute() {
    TestFixture tf;
    //affirm that if the motor is in the docking config and the mission manager doesn't want to undock, then the motor should remain in the docking config
    tf.docking_config_cmd_fp->set(true);
    tf.docking_controller->execute();
    TEST_ASSERT_EQUAL(true, tf.dock_config_fp->get());
    TEST_ASSERT_EQUAL(false, tf.docked_fp->get());
    //test docking
    tf.docked_fp->set(false);
    tf.dock_config_fp->set(false);
    while (!tf.dock_config_fp->get()){
        tf.docking_controller->execute();
    }
    TEST_ASSERT_EQUAL(true, tf.dock_config_fp->get());
    TEST_ASSERT_EQUAL(false, tf.docked_fp->get());
    //test undocking
    tf.docking_config_cmd_fp->set(false);
    while (tf.dock_config_fp->get()){
        tf.docking_controller->execute();
    }
    TEST_ASSERT_EQUAL(false, tf.dock_config_fp->get());
    TEST_ASSERT_EQUAL(false, tf.docked_fp->get());
}

int test_control_task() {
    UNITY_BEGIN();
    RUN_TEST(test_task_initialization);
    RUN_TEST(test_task_execute);
    return UNITY_END();
}

#ifdef DESKTOP
int main() {
    return test_control_task();
}

#else

#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_control_task();
}

#endif