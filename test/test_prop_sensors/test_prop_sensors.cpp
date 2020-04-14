#include "../test_prop/prop_common.h"

/**
 *
 * test_prop_sensors.cpp contains the hardware unit tests for the sensors used by the PropulsionSystem
 * - tank1 temp
 * - tank2 temp
 * - tank2 pressure
 */

using namespace Devices;

void test_setup()
{
    // Make sure that all the pins are correct
    TEST_ASSERT_EQUAL(Tank1.temp_sensor_pin, 21);
    TEST_ASSERT_EQUAL(Tank2.temp_sensor_pin, 22);
    TEST_ASSERT_EQUAL(Tank2.pressure_sensor_high_pin, 23);
    TEST_ASSERT_EQUAL(Tank2.pressure_sensor_low_pin, 20);
}

void test_temp_tank1()
{
    Serial.printf("Tank1 temperature: %d\n", Tank1.get_temp());
}

void test_temp_tank2()
{
    Serial.printf("Tank2 temperature: %d\n", Tank2.get_temp());
}

void test_pressure_tank2()
{
    Serial.printf("Tank2 pressure: %f\n", Tank2.get_pressure());
}


void setup() {
    Serial.begin(9600);
    pinMode(13, OUTPUT);
    while (!Serial)
        ;
    PropulsionSystem.setup();
    UNITY_BEGIN();
    RUN_TEST(test_setup);
    RUN_TEST(test_temp_tank1);
    RUN_TEST(test_temp_tank2);
    RUN_TEST(test_pressure_tank2);
    UNITY_END();
}

void loop()
{

}