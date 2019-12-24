#include "../StateFieldRegistryMock.hpp"
#include "../../src/FCCode/EEPROMController.hpp"
#include "../../src/FCCode/EEPROMController.cpp"
#include <EEPROM.h>

#include <unity.h>

class TestFixture {
  public:
    StateFieldRegistryMock registry;

    std::vector<std::string> statefields = {"pan.mode", "pan.deployed", "pan.sat_designation", "pan.cycle_no"}; 

    //Create the statefields that the EEPROM will eventually collect and store
    std::shared_ptr<ReadableStateField<unsigned int>> mission_mode_fp;
    std::shared_ptr<ReadableStateField<unsigned int>> is_deployed_fp;
    std::shared_ptr<ReadableStateField<unsigned int>> sat_designation_fp;
    std::shared_ptr<ReadableStateField<unsigned int>> control_cycle_count_fp;

    std::unique_ptr<EEPROMController> eeprom_controller;

    TestFixture() : registry() {
        mission_mode_fp = registry.create_readable_field<unsigned int>("pan.mode");
        mission_mode_fp->set(1);

        is_deployed_fp = registry.create_readable_field<unsigned int>("pan.deployed");
        is_deployed_fp->set(0);

        sat_designation_fp = registry.create_readable_field<unsigned int>("pan.sat_designation");
        sat_designation_fp->set(3);

        control_cycle_count_fp = registry.create_readable_field<unsigned int>("pan.cycle_no");
        control_cycle_count_fp->set(45);

        eeprom_controller = std::make_unique<EEPROMController>(registry, 0, statefields); 
    }
};

void test_task_initialization() {
    //Clear the EEPROM.
    #ifndef DESKTOP
    for (unsigned int i = 0 ; i < EEPROM.length() ; i++) {
        EEPROM.write(i, 255);
    }
    #endif

    TestFixture tf;

    for (size_t i = 0; i<tf.eeprom_controller->pointers.size(); i++){
        // Expect 1 0 3 45
        #ifndef DESKTOP
        Serial.println(tf.eeprom_controller->pointers.at(i)->get());
        #endif
    }

    TEST_ASSERT_EQUAL(1, tf.mission_mode_fp->get());
    TEST_ASSERT_EQUAL(0, tf.is_deployed_fp->get());
    TEST_ASSERT_EQUAL(3, tf.sat_designation_fp->get());
    TEST_ASSERT_EQUAL(45, tf.control_cycle_count_fp->get());
}

void test_task_execute() {
    TestFixture tf;

    // Let the statefields change over time
    tf.mission_mode_fp->set(2);
    tf.is_deployed_fp->set(0);
    tf.sat_designation_fp->set(2);
    tf.control_cycle_count_fp->set(50);

    // The period is set to 5. At the 50th control cycle, the EEPROM
    // should write the values to the EEPROM
    tf.eeprom_controller->execute();
    #ifndef DESKTOP
    for (size_t i = 0; i<tf.eeprom_controller->addresses.size(); i++){
        // Expect 2 0 2 50
        Serial.println(EEPROM.read(tf.eeprom_controller->addresses.at(i)));
    }
    #endif

    // Now we pretend the satellite just rebooted. Everytime the satellite reboots, another 
    // eeprom control task is instantiated.
    TestFixture tf2;

    // Check if the new eeprom controller set the statefield values to the values that 
    // were previously stored in the EEPROM
    for (unsigned int i = 0; i<tf2.eeprom_controller->pointers.size(); i++){
        // Expect 2 0 2 50
        Serial.println(tf2.eeprom_controller->pointers.at(i)->get());
    }

    // Now we let the statefield values change over time and let another period pass.
    tf2.mission_mode_fp->set(30);
    tf2.is_deployed_fp->set(1);
    tf2.sat_designation_fp->set(1);
    tf2.control_cycle_count_fp->set(55);

    // Check if those values were written to the EEPROM
    tf2.eeprom_controller->execute();
    #ifndef DESKTOP
    for (unsigned int i = 0; i<tf2.eeprom_controller->addresses.size(); i++){
        // Expect 30 1 1 55
        Serial.println(EEPROM.read(tf2.eeprom_controller->addresses.at(i)));
    }
    #endif

}

int test_control_task() {
    UNITY_BEGIN();
    RUN_TEST(test_task_initialization);
    RUN_TEST(test_task_execute);
    return UNITY_END();
}

#ifndef DESKTOP

#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    while (!Serial) {
    ; // wait for serial port to connect.
    }
    test_control_task();
}

void loop(){
    
}

#endif