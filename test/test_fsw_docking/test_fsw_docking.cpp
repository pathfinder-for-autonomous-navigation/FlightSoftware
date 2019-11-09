#include "../StateFieldRegistryMock.hpp"
#include <StateField.hpp>
#include <Serializer.hpp>
#include "../src/FCCode/DockingController.hpp"

#include <unity.h>

class TestFixture {
  public:
    StateFieldRegistryMock registry;
    Devices::DockingSystem docksys;

    std::shared_ptr<WritableStateField<bool>> docking_config_cmd_fp;

    std::unique_ptr<DockingController> docking_controller;

    std::shared_ptr<ReadableStateField<bool>>dock_config_fp;
    std::shared_ptr<ReadableStateField<bool>>docked_fp;
    std::shared_ptr<ReadableStateField<bool>>is_turning_fp;

    TestFixture() : registry() {
        docking_config_cmd_fp = registry.create_writable_field<bool>("docksys.config_cmd");
        docking_config_cmd_fp->set(true);

        docking_controller = std::make_unique<DockingController>(registry, docksys); 

        docked_fp = std::static_pointer_cast<ReadableStateField<bool>>(registry.find_readable_field("docksys.docked"));
        docked_fp->set(true);
        dock_config_fp = std::static_pointer_cast<ReadableStateField<bool>>(registry.find_readable_field("docksys.dock_config"));
        dock_config_fp->set(false);
        is_turning_fp = std::static_pointer_cast<ReadableStateField<bool>>(registry.find_readable_field("docksys.is_turning"));
        is_turning_fp->set(false);
    }
};

void test_task_initialization() {
    TestFixture tf;
    TEST_ASSERT_EQUAL(true, tf.docking_config_cmd_fp->get());
    TEST_ASSERT_EQUAL(true, tf.docked_fp->get());
    TEST_ASSERT_EQUAL(false, tf.dock_config_fp->get());
}

void test_task_execute() {
    TestFixture tf;
    tf.docked_fp->set(false);
    TEST_ASSERT_EQUAL(false, tf.docked_fp->get());
    //test docking
    while (!tf.dock_config_fp->get()){
        tf.docking_controller->execute();
    }
    TEST_ASSERT_EQUAL(true, tf.dock_config_fp->get());
    //test undocking
    tf.docking_config_cmd_fp->set(false);
    while (tf.dock_config_fp->get()){
        tf.docking_controller->execute();
    }
    TEST_ASSERT_EQUAL(false, tf.dock_config_fp->get());
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