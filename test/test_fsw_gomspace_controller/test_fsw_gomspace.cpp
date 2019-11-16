#include "../StateFieldRegistryMock.hpp"
#include "../../src/FCCode/GomspaceController.hpp"

#include <unity.h>

class TestFixture {
  public:
    StateFieldRegistryMock registry;
    Devices::Gomspace gs();

    std::unique_ptr<GomspaceController> gs_controller;

    ReadableStateField<unsigned int>* vboost1_fp;
    ReadableStateField<unsigned int>* vboost2_fp;
    ReadableStateField<unsigned int>* vboost3_fp;

    TestFixture() : registry() {
        gs_controller = std::make_unique<GomspaceController>(registry, 0, &gs);

        vboost1_fp = registry.find_readable_field_t<unsigned int>("gomspace.vboost.output1");
        vboost1_fp->set(7);

        vboost2_fp = registry.find_readable_field_t<unsigned int>("gomspace.vboost.output2");
        vboost2_fp->set(8);

        vboost3_fp = registry.find_readable_field_t<unsigned int>("gomspace.vboost.output3");
        vboost3_fp->set(9);
    }
};

void test_task_initialization() {
    TestFixture tf;
    TEST_ASSERT_EQUAL(7, tf.vboost1_fp->get());
    TEST_ASSERT_EQUAL(8, tf.vboost2_fp->get());
    TEST_ASSERT_EQUAL(9, tf.vboost3_fp->get());
}

void test_task_execute() {
    TestFixture tf;
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