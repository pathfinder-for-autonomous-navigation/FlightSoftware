#include "../StateFieldRegistryMock.hpp"

#include <fsw/FCCode/DCDCController.hpp>

#include "../custom_assertions.hpp"

class TestFixture {
  public:
    StateFieldRegistryMock registry;
    Devices::DCDC dcdc;

    std::unique_ptr<DCDCController> dcdc_controller;

    WritableStateField<bool>* ADCSMotorDCDC_cmd_fp;
    WritableStateField<bool>* SpikeDockDCDC_cmd_fp;
    WritableStateField<bool>* disable_cmd_fp;
    WritableStateField<bool>* reset_cmd_fp;
    ReadableStateField<bool>* ADCSMotorDCDC_fp;
    ReadableStateField<bool>* SpikeDockDCDC_fp;

    TestFixture() : registry(), dcdc("dcdc") {
        dcdc_controller = std::make_unique<DCDCController>(registry, dcdc);

        ADCSMotorDCDC_cmd_fp = registry.find_writable_field_t<bool>("dcdc.ADCSMotor_cmd");
        SpikeDockDCDC_cmd_fp = registry.find_writable_field_t<bool>("dcdc.SpikeDock_cmd");
        disable_cmd_fp = registry.find_writable_field_t<bool>("dcdc.disable_cmd");
        reset_cmd_fp = registry.find_writable_field_t<bool>("dcdc.reset_cmd");
        ADCSMotorDCDC_fp = registry.find_readable_field_t<bool>("dcdc.ADCSMotor");
        SpikeDockDCDC_fp = registry.find_readable_field_t<bool>("dcdc.SpikeDock");
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
    tf.ADCSMotorDCDC_cmd_fp->set(true);
    tf.SpikeDockDCDC_cmd_fp->set(true);
    tf.dcdc_controller->execute();
    TEST_ASSERT_EQUAL(true, tf.dcdc.adcs_enabled());
    TEST_ASSERT_EQUAL(true, tf.dcdc.sph_enabled());
    TEST_ASSERT_EQUAL(true, tf.ADCSMotorDCDC_cmd_fp->get());
    TEST_ASSERT_EQUAL(true, tf.SpikeDockDCDC_fp->get());

    tf.ADCSMotorDCDC_cmd_fp->set(false);
    tf.SpikeDockDCDC_cmd_fp->set(false);
    tf.dcdc_controller->execute();
    TEST_ASSERT_EQUAL(false, tf.dcdc.adcs_enabled());
    TEST_ASSERT_EQUAL(false, tf.ADCSMotorDCDC_cmd_fp->get());
    TEST_ASSERT_EQUAL(false, tf.SpikeDockDCDC_fp->get());
    TEST_ASSERT_EQUAL(false, tf.dcdc.sph_enabled());

    // Test the disable command
    tf.ADCSMotorDCDC_cmd_fp->set(true);
    tf.SpikeDockDCDC_cmd_fp->set(true);
    tf.dcdc_controller->execute();

    tf.disable_cmd_fp->set(true);
    tf.dcdc_controller->execute();
    TEST_ASSERT_EQUAL(false, tf.dcdc.adcs_enabled());
    TEST_ASSERT_EQUAL(false, tf.dcdc.sph_enabled());
    TEST_ASSERT_EQUAL(false, tf.ADCSMotorDCDC_cmd_fp->get());
    TEST_ASSERT_EQUAL(false, tf.SpikeDockDCDC_fp->get());
    TEST_ASSERT_EQUAL(false, tf.disable_cmd_fp->get());

    // If one pin is truned off and the other is on, then disable()
    // should still turn both pins off
    tf.ADCSMotorDCDC_cmd_fp->set(false);
    tf.SpikeDockDCDC_cmd_fp->set(true);
    tf.dcdc_controller->execute();
    TEST_ASSERT_EQUAL(false, tf.dcdc.adcs_enabled());
    TEST_ASSERT_EQUAL(true, tf.dcdc.sph_enabled());

    tf.disable_cmd_fp->set(true);
    tf.dcdc_controller->execute();
    TEST_ASSERT_EQUAL(false, tf.dcdc.adcs_enabled());
    TEST_ASSERT_EQUAL(false, tf.dcdc.sph_enabled());

    tf.ADCSMotorDCDC_cmd_fp->set(true);
    tf.SpikeDockDCDC_cmd_fp->set(false);
    tf.dcdc_controller->execute();
    TEST_ASSERT_EQUAL(true, tf.dcdc.adcs_enabled());
    TEST_ASSERT_EQUAL(false, tf.dcdc.sph_enabled());

    tf.disable_cmd_fp->set(true);
    tf.dcdc_controller->execute();
    TEST_ASSERT_EQUAL(false, tf.dcdc.adcs_enabled());
    TEST_ASSERT_EQUAL(false, tf.dcdc.sph_enabled());

    // The disable command shouldn't persist across control cycles if the DCDCs
    // are already disable
    tf.ADCSMotorDCDC_cmd_fp->set(false);
    tf.SpikeDockDCDC_cmd_fp->set(false);
    tf.dcdc_controller->execute();

    tf.disable_cmd_fp->set(true);
    tf.dcdc_controller->execute();
    TEST_ASSERT_EQUAL(false, tf.disable_cmd_fp->get());

    // Test the reset command
    tf.ADCSMotorDCDC_cmd_fp->set(true);
    tf.SpikeDockDCDC_cmd_fp->set(true);
    tf.dcdc_controller->execute();
    TEST_ASSERT_EQUAL(true, tf.dcdc.adcs_enabled());
    TEST_ASSERT_EQUAL(true, tf.dcdc.sph_enabled());

    tf.reset_cmd_fp->set(true);
    tf.dcdc_controller->execute();
    TEST_ASSERT_EQUAL(false, tf.dcdc.adcs_enabled());
    TEST_ASSERT_EQUAL(false, tf.dcdc.sph_enabled());
    TEST_ASSERT_EQUAL(true, tf.reset_cmd_fp->get());

    tf.dcdc_controller->execute();
    TEST_ASSERT_EQUAL(true, tf.dcdc.adcs_enabled());
    TEST_ASSERT_EQUAL(true, tf.dcdc.sph_enabled());
    TEST_ASSERT_EQUAL(false, tf.reset_cmd_fp->get());

    // If one pin is off and the other is on, then reset() should turn on 
    // both pins
    tf.ADCSMotorDCDC_cmd_fp->set(false);
    tf.SpikeDockDCDC_cmd_fp->set(true);
    tf.dcdc_controller->execute();
    TEST_ASSERT_EQUAL(false, tf.dcdc.adcs_enabled());
    TEST_ASSERT_EQUAL(true, tf.dcdc.sph_enabled());

    tf.reset_cmd_fp->set(true);
    tf.dcdc_controller->execute();
    TEST_ASSERT_EQUAL(false, tf.dcdc.adcs_enabled());
    TEST_ASSERT_EQUAL(false, tf.dcdc.sph_enabled());
    TEST_ASSERT_EQUAL(true, tf.reset_cmd_fp->get());

    tf.dcdc_controller->execute();
    TEST_ASSERT_EQUAL(true, tf.dcdc.adcs_enabled());
    TEST_ASSERT_EQUAL(true, tf.dcdc.sph_enabled());
    TEST_ASSERT_EQUAL(false, tf.reset_cmd_fp->get());

    tf.ADCSMotorDCDC_cmd_fp->set(true);
    tf.SpikeDockDCDC_cmd_fp->set(false);
    tf.dcdc_controller->execute();
    TEST_ASSERT_EQUAL(true, tf.dcdc.adcs_enabled());
    TEST_ASSERT_EQUAL(false, tf.dcdc.sph_enabled());

    tf.reset_cmd_fp->set(true);
    tf.dcdc_controller->execute();
    TEST_ASSERT_EQUAL(false, tf.dcdc.adcs_enabled());
    TEST_ASSERT_EQUAL(false, tf.dcdc.sph_enabled());
    TEST_ASSERT_EQUAL(true, tf.reset_cmd_fp->get());

    tf.dcdc_controller->execute();
    TEST_ASSERT_EQUAL(true, tf.dcdc.adcs_enabled());
    TEST_ASSERT_EQUAL(true, tf.dcdc.sph_enabled());
    TEST_ASSERT_EQUAL(false, tf.reset_cmd_fp->get());

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