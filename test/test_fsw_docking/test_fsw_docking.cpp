#include "../StateFieldRegistryMock.hpp"

#include <fsw/FCCode/DockingController.hpp>

#include <unity.h>

class TestFixture {
  public:
    StateFieldRegistryMock registry;
    Devices::DockingSystem docksys;

    std::shared_ptr<WritableStateField<bool>> docking_config_cmd_fp;

    std::unique_ptr<DockingController> docking_controller;

    ReadableStateField<bool>* dock_config_fp;
    ReadableStateField<bool>* docked_fp;
    ReadableStateField<bool>* is_turning_fp;

    TestFixture() : registry() {
        docking_config_cmd_fp = registry.create_writable_field<bool>("docksys.config_cmd");

        docking_controller = std::make_unique<DockingController>(registry, 0, docksys);
        docking_controller->init();

        docked_fp = registry.find_readable_field_t<bool>("docksys.docked");
        dock_config_fp = registry.find_readable_field_t<bool>("docksys.dock_config");
        is_turning_fp = registry.find_readable_field_t<bool>("docksys.is_turning");

        // Set initial values of external flags
        docking_config_cmd_fp->set(false);
    }
};

void test_task_initialization() {
    TestFixture tf;
    TEST_ASSERT_NOT_NULL(tf.docked_fp);
    TEST_ASSERT_NOT_NULL(tf.dock_config_fp);
    TEST_ASSERT_NOT_NULL(tf.is_turning_fp);

    TEST_ASSERT_FALSE(tf.docking_config_cmd_fp->get());
    TEST_ASSERT_FALSE(tf.docked_fp->get());
    TEST_ASSERT(tf.dock_config_fp->get());
    TEST_ASSERT_FALSE(tf.is_turning_fp->get());
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

void loop() {}
#endif