#include "../StateFieldRegistryMock.hpp"
#include <StateField.hpp>
#include <Serializer.hpp>
#include "../src/FCCode/DockingController.hpp"

#include <unity.h>

class TestFixture {
  public:

    StateFieldRegistryMock registry;
    //Input state fields to docking task
    std::shared_ptr<WritableStateField<float>> step_angle_fp;

    std::unique_ptr<DockingController> docking_task;
    //Output state fields from docking task
    std::shared_ptr<WritableStateField<bool>> is_turning_clockwise_fp;
    std::shared_ptr<WritableStateField<bool>> is_enabled_fp;

    TestFixture() : registry() {
        step_angle_fp = registry.create_writable_field<unsigned int>("step_angle", 0, 10, 10);
        step_angle_fp->set(3);

        docking_task = std::make_unique<DockingController>(registry);
        is_turning_clockwise_fp = std::static_pointer_cast<WritableStateField<unsigned int>>(registry.find_writable_field("is_turning_clockwise"));
        is_enabled_fp = std::static_pointer_cast<WritableStateField<unsigned int>>(registry.find_writable_field("is_enabled"));
    }
};

void test_task_initialization() {
    TestFixture tf;
    TEST_ASSERT_EQUAL(3, tf.step_angle_fp->get());
}

void test_task_execute() {
    TestFixture tf;
    TEST_ASSERT_EQUAL(0, tf.is_turning_clockwise_fp->get());
    tf.step_angle_fp->set(3);
    tf.docking_task->execute();
    TEST_ASSERT_EQUAL(3, tf.step_angle_fp->get());
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