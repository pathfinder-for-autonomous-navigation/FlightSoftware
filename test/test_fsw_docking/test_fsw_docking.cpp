#include "../StateFieldRegistryMock.hpp"
#include <StateField.hpp>
#include <Serializer.hpp>
#include "../src/FCCode/DockingController.hpp"

#include <unity.h>

class TestFixture {
  public:
    StateFieldRegistryMock registry;
    DockingSystem docksys;

    std::shared_ptr<WritableStateField<unsigned int>>is_turning_fp;
    std::shared_ptr<WritableStateField<unsigned int>>docking_mode_fp;
    std::unique_ptr<DockingController> docking_task;

    TestFixture() : registry() {
        is_turning_fp = registry.create_writable_field<unsigned int>("is_turning", 0, 10, 10);
        is_turning_fp->set(3);
        docking_mode_fp = registry.create_writable_field<unsigned int>("docking_mode", 0, 10, 10);
        docking_mode_fp->set(2);

        docking_task = std::make_unique<DockingController>(registry);
        is_turning_fp = std::static_pointer_cast<WritableStateField<unsigned int>>(registry.find_writable_field("is_turning"));
        docking_mode_fp = std::static_pointer_cast<WritableStateField<unsigned int>>(registry.find_writable_field("docking_mode"));
    }
};

void test_task_initialization() {
    TestFixture tf;
    TEST_ASSERT_EQUAL(3, tf.is_turning_fp->get());
}

void test_task_execute() {
    TestFixture tf;
    TEST_ASSERT_EQUAL(2, tf.docking_mode_fp->get());
    tf.docking_mode_fp->set(1);
    TEST_ASSERT_EQUAL(1, tf.docking_mode_fp->get());
    tf.docking_task->execute();
    TEST_ASSERT_EQUAL(1, tf.docking_mode_fp->get());
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