#include "../StateFieldRegistryMock.hpp"
#include "../../src/FCCode/EEPROMController.hpp"
#include "../../src/FCCode/EEPROMController.cpp"
#include <EEPROM.h>

#include <unity.h>

class TestFixture {
  public:
    StateFieldRegistryMock registry;

    std::shared_ptr<WritableStateField<unsigned char>> mission_mode_fp;
    std::shared_ptr<ReadableStateField<bool>> is_deployed_fp;
    std::shared_ptr<WritableStateField<unsigned char>> sat_designation_fp;
    std::shared_ptr<ReadableStateField<unsigned int>> control_cycle_count_fp;

    std::unique_ptr<EEPROMController> eeprom_controller;

    TestFixture() : registry() {
        mission_mode_fp = registry.create_writable_field<unsigned char>("pan.mode");
        mission_mode_fp->set(1);

        is_deployed_fp = registry.create_readable_field<bool>("pan.deployed");
        is_deployed_fp->set(false);

        sat_designation_fp = registry.create_writable_field<unsigned char>("pan.sat_designation");
        sat_designation_fp->set(3);

        control_cycle_count_fp = registry.create_readable_field<unsigned int>("pan.cycle_no");
        control_cycle_count_fp->set(45);

        eeprom_controller = std::make_unique<EEPROMController>(registry, 0); 
    }
};

void test_task_initialization() {
    //Clear the EEPROM.
    #ifndef DESKTOP
    for (int i = 0 ; i < EEPROM.length() ; i++) {
        EEPROM.write(i, 0);
    }
    #endif

    TestFixture tf;

    TEST_ASSERT_EQUAL(1, tf.mission_mode_fp->get());
    TEST_ASSERT_EQUAL(false, tf.is_deployed_fp->get());
    TEST_ASSERT_EQUAL(3, tf.sat_designation_fp->get());
    TEST_ASSERT_EQUAL(45, tf.control_cycle_count_fp->get());
}

void test_task_execute() {
    TestFixture tf;
    // The period is set to 5. At the 45th control cycle, the EEPROM
    // should write the values to the EEPROM
    tf.eeprom_controller->execute();
    #ifndef DESKTOP
    TEST_ASSERT_EQUAL(1, EEPROM.read(tf.eeprom_controller->mission_mode_address));
    TEST_ASSERT_EQUAL(false, EEPROM.read(tf.eeprom_controller->is_deployed_address));
    TEST_ASSERT_EQUAL(3, EEPROM.read(tf.eeprom_controller->sat_designation_address));
    TEST_ASSERT_EQUAL(45, EEPROM.read(tf.eeprom_controller->control_cycle_count_address));
    #endif

    // Now we have information stored in the EEPROM. If we change the statefield values and 
    // let another period pass, then we should store the new values in the EEPROM
    tf.mission_mode_fp->set(30);
    tf.is_deployed_fp->set(true);
    tf.sat_designation_fp->set(1);
    tf.control_cycle_count_fp->set(50);

    tf.eeprom_controller->execute();
    #ifndef DESKTOP
    TEST_ASSERT_EQUAL(30, EEPROM.read(tf.eeprom_controller->mission_mode_address));
    TEST_ASSERT_EQUAL(true, EEPROM.read(tf.eeprom_controller->is_deployed_address));
    TEST_ASSERT_EQUAL(1, EEPROM.read(tf.eeprom_controller->sat_designation_address));
    TEST_ASSERT_EQUAL(50, EEPROM.read(tf.eeprom_controller->control_cycle_count_address));
    #endif

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
    while (!Serial) {
    ; // wait for serial port to connect.
    }
    test_control_task();
    //Serial.println("hi!");
}

void loop(){
    //Serial.println("hello!");
}

#endif