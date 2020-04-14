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

}

void setup() {
    Serial.begin(9600);
    pinMode(13, OUTPUT);
    while (!Serial)
        ;
    PropulsionSystem.setup();
    UNITY_BEGIN();
    RUN_TEST(test_setup);
    UNITY_END();
}

void loop()
{

}