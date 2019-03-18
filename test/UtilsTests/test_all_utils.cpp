#include <unity_test/unity_fixture.h>
#include <Arduino.h>

// Weird bugfix required to fix linker error
extern "C"{
    int _getpid(){ return -1;}
    int _kill(int pid, int sig){ return -1; }
    int _write(){return -1;}
}

TEST_GROUP(UtilsTests);
TEST_SETUP(UtilsTests) {}
TEST_TEAR_DOWN(UtilsTests) {}

TEST_GROUP_RUNNER(UtilsTests) {
    RUN_TEST_GROUP(CircularBufferTests);
    RUN_TEST_GROUP(CircularStackTests);
    RUN_TEST_GROUP(CommsUtilsTests);
    RUN_TEST_GROUP(RWMutexTests);
}

void setup() {
    Serial.begin(9600);
    while(!Serial);
    UNITY_BEGIN();
    RUN_TEST_GROUP(UtilsTests);
    UNITY_END();
}

void loop() {}