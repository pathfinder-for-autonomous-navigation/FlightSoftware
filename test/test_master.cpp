#include <Arduino.h>
#include <unity_test/unity_fixture.h>

void setup() {
    while(!Serial);
    UNITY_BEGIN();
    RUN_TEST_GROUP(UtilsTests);
    UNITY_END();
    while(true);
}

void loop() {}