#include "../StateFieldRegistryMock.hpp"

#include <fsw/FCCode/EEPROMController.hpp>
#ifndef DESKTOP
#include <EEPROM.h>
#endif

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
    tf.eeprom_controller->init(tf.statefields);

    #ifndef DESKTOP
    TEST_ASSERT_EQUAL(1, tf.eeprom_controller->pointers.at(0)->get());
    TEST_ASSERT_EQUAL(0, tf.eeprom_controller->pointers.at(1)->get());
    TEST_ASSERT_EQUAL(3, tf.eeprom_controller->pointers.at(2)->get());
    TEST_ASSERT_EQUAL(45, tf.eeprom_controller->pointers.at(3)->get());
    #endif

    TEST_ASSERT_EQUAL(1, tf.mission_mode_fp->get());
    TEST_ASSERT_EQUAL(0, tf.is_deployed_fp->get());
    TEST_ASSERT_EQUAL(3, tf.sat_designation_fp->get());
    TEST_ASSERT_EQUAL(45, tf.control_cycle_count_fp->get());
}

void test_task_execute() {
    TestFixture tf;
    tf.eeprom_controller->init(tf.statefields);

    // Let the statefields change over time
    TimedControlTaskBase::control_cycle_count=50;
    tf.mission_mode_fp->set(2);
    tf.is_deployed_fp->set(0);
    tf.sat_designation_fp->set(2);
    tf.control_cycle_count_fp->set(50);

    // The period is set to 5. At the 50th control cycle, the EEPROM
    // should write the values to the EEPROM
    tf.eeprom_controller->execute();
    #ifndef DESKTOP
    TEST_ASSERT_EQUAL(2, EEPROM.read(tf.eeprom_controller->addresses.at(0)));
    TEST_ASSERT_EQUAL(0, EEPROM.read(tf.eeprom_controller->addresses.at(1)));
    TEST_ASSERT_EQUAL(2, EEPROM.read(tf.eeprom_controller->addresses.at(2)));
    TEST_ASSERT_EQUAL(50, EEPROM.read(tf.eeprom_controller->addresses.at(3)));
    #endif

    // Now we pretend the satellite just rebooted. Everytime the satellite reboots, another 
    // eeprom control task is instantiated.
    TestFixture tf2;
    tf2.eeprom_controller->init(tf2.statefields);

    // Check if the new eeprom controller set the statefield values to the values that 
    // were previously stored in the EEPROM
    #ifndef DESKTOP
    TEST_ASSERT_EQUAL(2, tf2.eeprom_controller->pointers.at(0)->get());
    TEST_ASSERT_EQUAL(0, tf2.eeprom_controller->pointers.at(1)->get());
    TEST_ASSERT_EQUAL(2, tf2.eeprom_controller->pointers.at(2)->get());
    TEST_ASSERT_EQUAL(50, tf2.eeprom_controller->pointers.at(3)->get());
    #endif

    // Now we let the statefield values change over time and let another period pass.
    TimedControlTaskBase::control_cycle_count=55;
    tf2.mission_mode_fp->set(30);
    tf2.is_deployed_fp->set(1);
    tf2.sat_designation_fp->set(1);
    tf2.control_cycle_count_fp->set(55);

    // Check if those values were written to the EEPROM
    tf2.eeprom_controller->execute();
    #ifndef DESKTOP
    TEST_ASSERT_EQUAL(30, EEPROM.read(tf2.eeprom_controller->addresses.at(0)));
    TEST_ASSERT_EQUAL(1, EEPROM.read(tf2.eeprom_controller->addresses.at(1)));
    TEST_ASSERT_EQUAL(1, EEPROM.read(tf2.eeprom_controller->addresses.at(2)));
    TEST_ASSERT_EQUAL(55, EEPROM.read(tf2.eeprom_controller->addresses.at(3)));
    #endif

    // Now we let a few more control cycles pass, but not a whole period
    TimedControlTaskBase::control_cycle_count=57;
    tf2.mission_mode_fp->set(28);
    tf2.is_deployed_fp->set(0);
    tf2.sat_designation_fp->set(6);
    tf2.control_cycle_count_fp->set(57);

    // Check that these values are NOT written to the EEPROM
    tf2.eeprom_controller->execute();
    #ifndef DESKTOP
    TEST_ASSERT_EQUAL(30, EEPROM.read(tf2.eeprom_controller->addresses.at(0)));
    TEST_ASSERT_EQUAL(1, EEPROM.read(tf2.eeprom_controller->addresses.at(1)));
    TEST_ASSERT_EQUAL(1, EEPROM.read(tf2.eeprom_controller->addresses.at(2)));
    TEST_ASSERT_EQUAL(55, EEPROM.read(tf2.eeprom_controller->addresses.at(3)));
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
}

void loop(){
    
}

#endif