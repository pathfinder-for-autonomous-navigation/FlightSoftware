#include <Arduino.h>
#include <unity_test/unity_fixture.h>

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST_GROUP(FCDeviceTests);
    RUN_TEST_GROUP(FCSoftwareTests);
    RUN_TEST_GROUP(UtilsTests);
    //RUN_TEST_GROUP(ADCSDeviceTests);
    //RUN_TEST_GROUP(ADCSSoftwareTests);
    UNITY_END();
}

void loop() {}