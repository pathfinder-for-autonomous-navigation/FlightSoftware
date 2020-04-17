#include "../StateFieldRegistryMock.hpp"

#include <fsw/FCCode/EEPROMController.hpp>

#include <unity.h>

#ifdef DESKTOP
    #include <json.hpp>
    #include <fstream>
#else
    #include <EEPROM.h>
#endif

class TestFixture {
  public:
    StateFieldRegistryMock registry;

    //Create the statefields that the EEPROM will eventually collect and store
    std::shared_ptr<ReadableStateField<unsigned char>> mission_mode_fp;
    std::shared_ptr<ReadableStateField<bool>> is_deployed_fp;
    std::shared_ptr<ReadableStateField<unsigned char>> sat_designation_fp;
    std::shared_ptr<ReadableStateField<unsigned int>> control_cycle_count_fp;

    std::unique_ptr<EEPROMController> eeprom_controller;

    /**
     * @brief Construct a new Test Fixture.
     * 
     * @param clr If true, clears the EEPROM.
     */
    TestFixture(bool clr) : registry() {
        if (clr) clear_data();

        mission_mode_fp = registry.create_readable_field<unsigned char, 2>("pan.state");
        mission_mode_fp->set(1);

        is_deployed_fp = registry.create_readable_field<bool, 3>("pan.deployed");
        is_deployed_fp->set(false);

        sat_designation_fp = registry.create_readable_field<unsigned char, 5>("pan.sat_designation");
        sat_designation_fp->set(3);

        control_cycle_count_fp = registry.create_readable_field<unsigned int, 7>("pan.cycle_no");
        control_cycle_count_fp->set(4);

        eeprom_controller = std::make_unique<EEPROMController>(registry, 0);

        // Initialize the controller
        eeprom_controller->init();
    }

    /**
     * @brief Clear data from the EEPROM.
     */
    void clear_data() {
        #ifdef DESKTOP
            EEPROMController::data.clear();
            save_data();
        #else
            for (unsigned int i = 0 ; i < EEPROM.length() ; i++) {
                EEPROM.write(i, 255);
            }
        #endif
    }

    /**
     * @brief Save data to the EEPROM. Only does anything in desktop mode.
     */
    void save_data() {
        #ifdef DESKTOP
            EEPROMController::save_data(0);
        #endif
    }

    /**
     * Get JSON data stored in the EEPROM data file. Desktop-only.
     */
    #ifdef DESKTOP
    nlohmann::json read_EEPROM() {
        eeprom_controller->read_EEPROM();
        return EEPROMController::data;
    }
    #endif

    /**
     * @brief Reads data from EEPROM for the statefield at index idx.
     * 
     * @param idx 
     * @return unsigned int 
     */
    unsigned int read(size_t idx) {
        #ifdef DESKTOP
            const std::string& field_name = registry.eeprom_saved_fields[idx]->name();
            if (EEPROMController::data.find(field_name) != EEPROMController::data.end()) {
                return EEPROMController::data[field_name];
            }
            else {
                return 255;
            }
        #else
            return EEPROM.read(eeprom_controller->addresses[idx]);
        #endif
    }

    /**
     * @brief Gets pointer to statefield at index idx. 
     */
    template<typename T>
    ReadableStateField<T>* get_ptr(size_t idx) {
        static_assert(SerializableStateField<T>::is_eeprom_saveable(),
            "Cannot return a pointer to a non-EEPROM saveable field.");
        return static_cast<ReadableStateField<T>*>(registry.eeprom_saved_fields[idx]);
    }
};

void test_task_initialization() {
    TestFixture tf(true);

    TEST_ASSERT_EQUAL(1, tf.get_ptr<unsigned char>(0)->get());
    TEST_ASSERT_FALSE(tf.get_ptr<bool>(1)->get());
    TEST_ASSERT_EQUAL(3, tf.get_ptr<unsigned char>(2)->get());
    TEST_ASSERT_EQUAL(4, tf.get_ptr<unsigned int>(3)->get());

    TEST_ASSERT_EQUAL(1, tf.mission_mode_fp->get());
    TEST_ASSERT_FALSE(tf.is_deployed_fp->get());
    TEST_ASSERT_EQUAL(3, tf.sat_designation_fp->get());
    TEST_ASSERT_EQUAL(4, tf.control_cycle_count_fp->get());
}

void test_task_execute() {
    TestFixture tf(true);

    // Set the statefields to new values.
    tf.mission_mode_fp->set(5);
    tf.is_deployed_fp->set(true);
    tf.sat_designation_fp->set(7);
    tf.control_cycle_count_fp->set(8);

    // Test that each of the values are written after the appropriate number of control cycles has passed
    TimedControlTaskBase::control_cycle_count=4;

    // At the 4th control cycle, the EEPROM should write the value of mission mode (period=2)
    // All the other addresses should hold the default value, we haven't written anything there yet.
    tf.eeprom_controller->execute();
    TEST_ASSERT_EQUAL(5, tf.read(0));
    TEST_ASSERT_EQUAL(255, tf.read(1));
    TEST_ASSERT_EQUAL(255, tf.read(2));
    TEST_ASSERT_EQUAL(255, tf.read(3));

    // At the 9th control cycle, the EEPROM should write the value of deployment mode (period=3)
    TimedControlTaskBase::control_cycle_count=9;
    tf.eeprom_controller->execute();
    TEST_ASSERT_EQUAL(5, tf.read(0));
    TEST_ASSERT_EQUAL(1, tf.read(1));
    TEST_ASSERT_EQUAL(255, tf.read(2));
    TEST_ASSERT_EQUAL(255, tf.read(3));

    // At the 25th control cycle, the EEPROM should write the value of sat designation (period=5)
    TimedControlTaskBase::control_cycle_count=25;
    tf.eeprom_controller->execute();
    TEST_ASSERT_EQUAL(5, tf.read(0));
    TEST_ASSERT_EQUAL(1, tf.read(1));
    TEST_ASSERT_EQUAL(7, tf.read(2));
    TEST_ASSERT_EQUAL(255, tf.read(3));

    // At the 49th control cycle, the EEPROM should write the value of cycle number (period=7)
    TimedControlTaskBase::control_cycle_count=49;
    tf.eeprom_controller->execute();
    TEST_ASSERT_EQUAL(5, tf.read(0));
    TEST_ASSERT_EQUAL(1, tf.read(1));
    TEST_ASSERT_EQUAL(7, tf.read(2));
    TEST_ASSERT_EQUAL(8, tf.read(3));

    // Now we pretend the satellite just rebooted. Everytime the satellite reboots, another 
    // eeprom control task is instantiated.
    tf.save_data();
    TestFixture tf2(false);

    // Check if the new eeprom controller set the statefield values to the values that 
    // were previously stored in the EEPROM
    TEST_ASSERT_EQUAL(5, tf2.get_ptr<unsigned char>(0)->get());
    TEST_ASSERT_TRUE(tf2.get_ptr<bool>(1)->get());
    TEST_ASSERT_EQUAL(7, tf2.get_ptr<unsigned char>(2)->get());
    TEST_ASSERT_EQUAL(8, tf2.get_ptr<unsigned int>(3)->get());

    // Let the statefield values change over time.
    tf2.mission_mode_fp->set(9);
    tf2.is_deployed_fp->set(false);
    tf2.sat_designation_fp->set(11);
    tf2.control_cycle_count_fp->set(12);

    // At the 210th control cycle, all the new values should be written to the EEPROM (210=2*3*5*7)
    TimedControlTaskBase::control_cycle_count=210;

    // Check if those values were written to the EEPROM
    tf2.eeprom_controller->execute();
    TEST_ASSERT_EQUAL(9, tf.read(0));
    TEST_ASSERT_EQUAL(0, tf.read(1));
    TEST_ASSERT_EQUAL(11, tf.read(2));
    TEST_ASSERT_EQUAL(12, tf.read(3));

    // Now we let a few more control cycles pass, but not a whole period for any statefield (211 is a prime number)
    TimedControlTaskBase::control_cycle_count=211;
    tf2.mission_mode_fp->set(13);
    tf2.is_deployed_fp->set(true);
    tf2.sat_designation_fp->set(15);
    tf2.control_cycle_count_fp->set(16);

    // Check that these values are NOT written to the EEPROM
    tf2.eeprom_controller->execute();
    TEST_ASSERT_EQUAL(9, tf.read(0));
    TEST_ASSERT_EQUAL(0, tf.read(1));
    TEST_ASSERT_EQUAL(11, tf.read(2));
    TEST_ASSERT_EQUAL(12, tf.read(3));
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
        delay(10000);
        Serial.begin(9600);
        test_control_task();
    }

    void loop(){}
#endif
