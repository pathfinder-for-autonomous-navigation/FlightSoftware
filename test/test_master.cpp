#include <Arduino.h>
#include <unity_test/unity_fixture.h>

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST_GROUP(FCSoftwareTests);
    RUN_TEST_GROUP(UtilsTests);
    UNITY_END();
}

void loop() {}