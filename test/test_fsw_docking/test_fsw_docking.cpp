#include "../StateFieldRegistryMock.hpp"
#include <StateField.hpp>
#include <Serializer.hpp>
#include "../src/FCCode/DockingController.hpp"

#include <unity.h>

class TestFixture {
  public:
    StateFieldRegistryMock registry;
    std::shared_ptr<WritableStateField<unsigned int>>check_mode_fp;

    std::unique_ptr<DockingController> docking_task;
    std::shared_ptr<WritableStateField<unsigned int>>set_mode_fp;

    TestFixture() : registry() {
        set_mode_fp = registry.create_writable_field<unsigned int>("set_mode", 0, 10, 10);
        set_mode_fp->set(3);

        docking_task = std::make_unique<DockingController>(registry);
        check_mode_fp = std::static_pointer_cast<WritableStateField<unsigned int>>(registry.find_writable_field("check_mode"));
    }
};

void test_task_initialization() {
    TestFixture tf;
    TEST_ASSERT_EQUAL(3, tf.set_mode_fp->get());
}

void test_task_execute() {
    TestFixture tf;
    TEST_ASSERT_EQUAL(0, tf.check_mode_fp->get());
    tf.set_mode_fp->set(1);
    tf.docking_task->execute();
    TEST_ASSERT_EQUAL(1, tf.check_mode_fp->get());
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