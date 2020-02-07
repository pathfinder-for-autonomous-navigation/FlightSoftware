#include "../StateFieldRegistryMock.hpp"

#include <fsw/FCCode/DCDCController.hpp>

#include <unity.h>

class TestFixture {
  public:
    StateFieldRegistryMock registry;
    Devices::DCDC dcdc;

    std::unique_ptr<DCDCController> dcdc_controller;

    WritableStateField<bool>* adcs_dcdc_cmd_fp;
    WritableStateField<bool>* sph_dcdc_cmd_fp;
    WritableStateField<bool>* disable_cmd_fp;
    WritableStateField<bool>* reset_cmd_fp;

    TestFixture() : registry(), dcdc("dcdc") {
        dcdc_controller = std::make_unique<DCDCController>(registry, 0, dcdc);

        adcs_dcdc_cmd_fp = registry.find_writable_field_t<bool>("dcdc.adcs_cmd");
        sph_dcdc_cmd_fp = registry.find_writable_field_t<bool>("dcdc.sph_cmd");
        disable_cmd_fp = registry.find_writable_field_t<bool>("dcdc.disable_cmd");
        reset_cmd_fp = registry.find_writable_field_t<bool>("dcdc.reset_cmd");
    }
};

void test_task_initialization() {
    TestFixture tf;
    TEST_ASSERT_NOT_NULL(tf.adcs_dcdc_cmd_fp);
    TEST_ASSERT_NOT_NULL(tf.sph_dcdc_cmd_fp);
    TEST_ASSERT_NOT_NULL(tf.disable_cmd_fp);
    TEST_ASSERT_NOT_NULL(tf.reset_cmd_fp);

    TEST_ASSERT_FALSE(tf.adcs_dcdc_cmd_fp->get());
    TEST_ASSERT_FALSE(tf.sph_dcdc_cmd_fp->get());
    TEST_ASSERT_FALSE(tf.disable_cmd_fp->get());
    TEST_ASSERT_FALSE(tf.reset_cmd_fp->get());
}

void test_task_execute() {
    TestFixture tf;
    tf.adcs_dcdc_cmd_fp->set(true);
    tf.sph_dcdc_cmd_fp->set(true);
    tf.dcdc_controller->execute();
    TEST_ASSERT_EQUAL(true, tf.dcdc.adcs_enabled());
    TEST_ASSERT_EQUAL(true, tf.dcdc.sph_enabled());

    tf.adcs_dcdc_cmd_fp->set(false);
    tf.sph_dcdc_cmd_fp->set(false);
    tf.dcdc_controller->execute();
    TEST_ASSERT_EQUAL(false, tf.dcdc.adcs_enabled());
    TEST_ASSERT_EQUAL(false, tf.dcdc.sph_enabled());

    tf.adcs_dcdc_cmd_fp->set(true);
    tf.sph_dcdc_cmd_fp->set(true);

    tf.disable_cmd_fp->set(true);
    tf.dcdc_controller->execute();
    TEST_ASSERT_EQUAL(false, tf.dcdc.adcs_enabled());
    TEST_ASSERT_EQUAL(false, tf.dcdc.sph_enabled());

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