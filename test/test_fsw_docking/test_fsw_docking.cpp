#include "../StateFieldRegistryMock.hpp"
#include <StateField.hpp>
#include <Serializer.hpp>
#include "../src/FCCode/DockingController.hpp"

#include <unity.h>

class TestFixture {
  public:
    StateFieldRegistryMock registry;
    DockingSystem docksys;

    std::shared_ptr<WritableStateField<bool>> docking_motor_dock_fp;

    std::unique_ptr<DockingController> docking_controller;

    std::shared_ptr<ReadableStateField<bool>>is_turning_fp;
    std::shared_ptr<ReadableStateField<bool>>docked_fp;

    TestFixture() : registry() {
        docking_motor_dock_fp = registry.create_writable_field<bool>("docking_motor_dock");
        docking_motor_dock_fp->set(false);

        docking_controller = std::make_unique<DockingController>(registry); 

        docked_fp = std::static_pointer_cast<ReadableStateField<bool>>(registry.find_readable_field("docked"));
        is_turning_fp = std::static_pointer_cast<ReadableStateField<bool>>(registry.find_readable_field("is_turning"));
    }
};

void test_task_initialization() {
    TestFixture tf;
    TEST_ASSERT_EQUAL(false, tf.docking_motor_dock_fp->get());
}

void test_task_execute() {
    TestFixture tf;
    TEST_ASSERT_EQUAL(true, tf.docked_fp->get());
    tf.docked_fp->set(false);
    TEST_ASSERT_EQUAL(false, tf.docked_fp->get());
    tf.docking_controller->execute();
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
#endif