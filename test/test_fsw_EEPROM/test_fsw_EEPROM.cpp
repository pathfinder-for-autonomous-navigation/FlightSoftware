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
    std::vector<unsigned int> periods = {2, 3, 5, 7};

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
        is_deployed_fp->set(2);

        sat_designation_fp = registry.create_readable_field<unsigned int>("pan.sat_designation");
        sat_designation_fp->set(3);

        control_cycle_count_fp = registry.create_readable_field<unsigned int>("pan.cycle_no");
        control_cycle_count_fp->set(4);

        eeprom_controller = std::make_unique<EEPROMController>(registry, 0); 
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
    tf.eeprom_controller->init(tf.statefields, tf.periods);

    #ifndef DESKTOP
    TEST_ASSERT_EQUAL(1, tf.eeprom_controller->pointers.at(0)->get());
    TEST_ASSERT_EQUAL(2, tf.eeprom_controller->pointers.at(1)->get());
    TEST_ASSERT_EQUAL(3, tf.eeprom_controller->pointers.at(2)->get());
    TEST_ASSERT_EQUAL(4, tf.eeprom_controller->pointers.at(3)->get());
    #endif

    TEST_ASSERT_EQUAL(1, tf.mission_mode_fp->get());
    TEST_ASSERT_EQUAL(2, tf.is_deployed_fp->get());
    TEST_ASSERT_EQUAL(3, tf.sat_designation_fp->get());
    TEST_ASSERT_EQUAL(4, tf.control_cycle_count_fp->get());
}

void test_task_execute() {
    TestFixture tf;
    tf.eeprom_controller->init(tf.statefields, tf.periods);

    // Set the statefields to new values.
    tf.mission_mode_fp->set(5);
    tf.is_deployed_fp->set(6);
    tf.sat_designation_fp->set(7);
    tf.control_cycle_count_fp->set(8);

    // Test that each of the values are written after the appropriate number of control cycles has passed
    TimedControlTaskBase::control_cycle_count=4;

    // At the 4th control cycle, the EEPROM should write the value of mission mode (period=2)
    // All the other addresses should hold the default value, we haven't written anything there yet.
    tf.eeprom_controller->execute();
    #ifndef DESKTOP
    TEST_ASSERT_EQUAL(5, EEPROM.read(tf.eeprom_controller->addresses.at(0)));
    TEST_ASSERT_EQUAL(255, EEPROM.read(tf.eeprom_controller->addresses.at(1)));
    TEST_ASSERT_EQUAL(255, EEPROM.read(tf.eeprom_controller->addresses.at(2)));
    TEST_ASSERT_EQUAL(255, EEPROM.read(tf.eeprom_controller->addresses.at(3)));
    #endif

    // At the 9th control cycle, the EEPROM should write the value of deployment mode (period=3)
    TimedControlTaskBase::control_cycle_count=9;
    tf.eeprom_controller->execute();
    #ifndef DESKTOP
    TEST_ASSERT_EQUAL(5, EEPROM.read(tf.eeprom_controller->addresses.at(0)));
    TEST_ASSERT_EQUAL(6, EEPROM.read(tf.eeprom_controller->addresses.at(1)));
    TEST_ASSERT_EQUAL(255, EEPROM.read(tf.eeprom_controller->addresses.at(2)));
    TEST_ASSERT_EQUAL(255, EEPROM.read(tf.eeprom_controller->addresses.at(3)));
    #endif

    // At the 25th control cycle, the EEPROM should write the value of sat designation (period=5)
    TimedControlTaskBase::control_cycle_count=25;
    tf.eeprom_controller->execute();
    #ifndef DESKTOP
    TEST_ASSERT_EQUAL(5, EEPROM.read(tf.eeprom_controller->addresses.at(0)));
    TEST_ASSERT_EQUAL(6, EEPROM.read(tf.eeprom_controller->addresses.at(1)));
    TEST_ASSERT_EQUAL(7, EEPROM.read(tf.eeprom_controller->addresses.at(2)));
    TEST_ASSERT_EQUAL(255, EEPROM.read(tf.eeprom_controller->addresses.at(3)));
    #endif

    // At the 49th control cycle, the EEPROM should write the value of cycle number (period=7)
    TimedControlTaskBase::control_cycle_count=49;
    tf.eeprom_controller->execute();
    #ifndef DESKTOP
    TEST_ASSERT_EQUAL(5, EEPROM.read(tf.eeprom_controller->addresses.at(0)));
    TEST_ASSERT_EQUAL(6, EEPROM.read(tf.eeprom_controller->addresses.at(1)));
    TEST_ASSERT_EQUAL(7, EEPROM.read(tf.eeprom_controller->addresses.at(2)));
    TEST_ASSERT_EQUAL(8, EEPROM.read(tf.eeprom_controller->addresses.at(3)));
    #endif

    // Now we pretend the satellite just rebooted. Everytime the satellite reboots, another 
    // eeprom control task is instantiated.
    TestFixture tf2;
    tf2.eeprom_controller->init(tf2.statefields, tf2.periods);

    // Check if the new eeprom controller set the statefield values to the values that 
    // were previously stored in the EEPROM
    #ifndef DESKTOP
    TEST_ASSERT_EQUAL(5, tf2.eeprom_controller->pointers.at(0)->get());
    TEST_ASSERT_EQUAL(6, tf2.eeprom_controller->pointers.at(1)->get());
    TEST_ASSERT_EQUAL(7, tf2.eeprom_controller->pointers.at(2)->get());
    TEST_ASSERT_EQUAL(8, tf2.eeprom_controller->pointers.at(3)->get());
    #endif

    // Let the statefield values change over time.
    tf2.mission_mode_fp->set(9);
    tf2.is_deployed_fp->set(10);
    tf2.sat_designation_fp->set(11);
    tf2.control_cycle_count_fp->set(12);

    // At the 210th control cycle, all the new values should be written to the EEPROM (210=2*3*5*7)
    TimedControlTaskBase::control_cycle_count=210;

    // Check if those values were written to the EEPROM
    tf2.eeprom_controller->execute();
    #ifndef DESKTOP
    TEST_ASSERT_EQUAL(9, EEPROM.read(tf2.eeprom_controller->addresses.at(0)));
    TEST_ASSERT_EQUAL(10, EEPROM.read(tf2.eeprom_controller->addresses.at(1)));
    TEST_ASSERT_EQUAL(11, EEPROM.read(tf2.eeprom_controller->addresses.at(2)));
    TEST_ASSERT_EQUAL(12, EEPROM.read(tf2.eeprom_controller->addresses.at(3)));
    #endif

    // Now we let a few more control cycles pass, but not a whole period for any statefield (211 is a prime number)
    TimedControlTaskBase::control_cycle_count=211;
    tf2.mission_mode_fp->set(13);
    tf2.is_deployed_fp->set(14);
    tf2.sat_designation_fp->set(15);
    tf2.control_cycle_count_fp->set(16);

    // Check that these values are NOT written to the EEPROM
    tf2.eeprom_controller->execute();
    #ifndef DESKTOP
    TEST_ASSERT_EQUAL(9, EEPROM.read(tf2.eeprom_controller->addresses.at(0)));
    TEST_ASSERT_EQUAL(10, EEPROM.read(tf2.eeprom_controller->addresses.at(1)));
    TEST_ASSERT_EQUAL(11, EEPROM.read(tf2.eeprom_controller->addresses.at(2)));
    TEST_ASSERT_EQUAL(12, EEPROM.read(tf2.eeprom_controller->addresses.at(3)));
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
    Serial.println(test_control_task());
}

void loop(){
    
}

#endif