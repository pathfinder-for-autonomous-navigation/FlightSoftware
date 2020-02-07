#include "../StateFieldRegistryMock.hpp"

#include <fsw/FCCode/DCDCController.hpp>

#include <unity.h>

class TestFixture {
  public:
    StateFieldRegistryMock registry;
    Devices::DCDC dcdc;

    std::unique_ptr<DCDCController> dcdc_controller;

    WritableStateField<bool>* ADCSMotorDCDC_fp;
    WritableStateField<bool>* SpikeDockDCDC_fp;
    WritableStateField<bool>* disable_cmd_fp;
    WritableStateField<bool>* reset_cmd_fp;

    TestFixture() : registry(), dcdc("dcdc") {
        dcdc_controller = std::make_unique<DCDCController>(registry, 0, dcdc);

        ADCSMotorDCDC_fp = registry.find_writable_field_t<bool>("dcdc.ADCSMotor");
        SpikeDockDCDC_fp = registry.find_writable_field_t<bool>("dcdc.SpikeDock");
        disable_cmd_fp = registry.find_writable_field_t<bool>("dcdc.disable_cmd");
        reset_cmd_fp = registry.find_writable_field_t<bool>("dcdc.reset_cmd");
    }
};

void test_task_initialization() {
    TestFixture tf;
    TEST_ASSERT_NOT_NULL(tf.ADCSMotorDCDC_fp);
    TEST_ASSERT_NOT_NULL(tf.SpikeDockDCDC_fp);
    TEST_ASSERT_NOT_NULL(tf.disable_cmd_fp);
    TEST_ASSERT_NOT_NULL(tf.reset_cmd_fp);

    TEST_ASSERT_FALSE(tf.ADCSMotorDCDC_fp->get());
    TEST_ASSERT_FALSE(tf.SpikeDockDCDC_fp->get());
    TEST_ASSERT_FALSE(tf.disable_cmd_fp->get());
    TEST_ASSERT_FALSE(tf.reset_cmd_fp->get());
}

void test_task_execute() {
    TestFixture tf;

    // Test enabling/disabling the individual ADCS and SPH pins
    tf.ADCSMotorDCDC_fp->set(true);
    tf.SpikeDockDCDC_fp->set(true);
    tf.dcdc_controller->execute();
    TEST_ASSERT_EQUAL(true, tf.dcdc.adcs_enabled());
    TEST_ASSERT_EQUAL(true, tf.dcdc.sph_enabled());

    tf.ADCSMotorDCDC_fp->set(false);
    tf.SpikeDockDCDC_fp->set(false);
    tf.dcdc_controller->execute();
    TEST_ASSERT_EQUAL(false, tf.dcdc.adcs_enabled());
    TEST_ASSERT_EQUAL(false, tf.dcdc.sph_enabled());

    // Test the disable command
    tf.ADCSMotorDCDC_fp->set(true);
    tf.SpikeDockDCDC_fp->set(true);

    tf.disable_cmd_fp->set(true);
    tf.dcdc_controller->execute();
    TEST_ASSERT_EQUAL(false, tf.dcdc.adcs_enabled());
    TEST_ASSERT_EQUAL(false, tf.dcdc.sph_enabled());

    // If one pin is truned off and the other is on, then disable()
    // should still turn both pins off
    tf.ADCSMotorDCDC_fp->set(false);
    tf.SpikeDockDCDC_fp->set(true);
    tf.dcdc_controller->execute();
    TEST_ASSERT_EQUAL(false, tf.dcdc.adcs_enabled());
    TEST_ASSERT_EQUAL(true, tf.dcdc.sph_enabled());

    tf.disable_cmd_fp->set(true);
    tf.dcdc_controller->execute();
    TEST_ASSERT_EQUAL(false, tf.dcdc.adcs_enabled());
    TEST_ASSERT_EQUAL(false, tf.dcdc.sph_enabled());

    tf.ADCSMotorDCDC_fp->set(true);
    tf.SpikeDockDCDC_fp->set(false);
    tf.dcdc_controller->execute();
    TEST_ASSERT_EQUAL(true, tf.dcdc.adcs_enabled());
    TEST_ASSERT_EQUAL(false, tf.dcdc.sph_enabled());

    tf.disable_cmd_fp->set(true);
    tf.dcdc_controller->execute();
    TEST_ASSERT_EQUAL(false, tf.dcdc.adcs_enabled());
    TEST_ASSERT_EQUAL(false, tf.dcdc.sph_enabled());

    // Test the reset command
    tf.reset_cmd_fp->set(true);
    tf.dcdc_controller->execute();
    TEST_ASSERT_EQUAL(true, tf.dcdc.adcs_enabled());
    TEST_ASSERT_EQUAL(true, tf.dcdc.sph_enabled());

    // If one pin is off and the other is on, then reset() should turn on 
    // both pins
    tf.ADCSMotorDCDC_fp->set(false);
    tf.SpikeDockDCDC_fp->set(true);
    tf.dcdc_controller->execute();
    TEST_ASSERT_EQUAL(false, tf.dcdc.adcs_enabled());
    TEST_ASSERT_EQUAL(true, tf.dcdc.sph_enabled());

    tf.reset_cmd_fp->set(true);
    tf.dcdc_controller->execute();
    TEST_ASSERT_EQUAL(true, tf.dcdc.adcs_enabled());
    TEST_ASSERT_EQUAL(true, tf.dcdc.sph_enabled());

    tf.ADCSMotorDCDC_fp->set(true);
    tf.SpikeDockDCDC_fp->set(false);
    tf.dcdc_controller->execute();
    TEST_ASSERT_EQUAL(true, tf.dcdc.adcs_enabled());
    TEST_ASSERT_EQUAL(false, tf.dcdc.sph_enabled());

    tf.reset_cmd_fp->set(true);
    tf.dcdc_controller->execute();
    TEST_ASSERT_EQUAL(true, tf.dcdc.adcs_enabled());
    TEST_ASSERT_EQUAL(true, tf.dcdc.sph_enabled());

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