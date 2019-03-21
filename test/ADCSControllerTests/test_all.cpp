#include <unity_fixture.h>
#include <ChRt.h>
#include <rwmutex.hpp>
#include <Arduino.h>

// Weird bugfix required to fix linker error
extern "C"{
    int _getpid(){ return -1;}
    int _kill(int pid, int sig){ return -1; }
    int _write(){return -1;}
}

TEST_GROUP(ADCSControllerTests);
TEST_SETUP(ADCSControllerTests) {}
TEST_TEAR_DOWN(ADCSControllerTests) {}

TEST_GROUP_RUNNER(ADCSControllerTests) {
    RUN_TEST_GROUP(ADCSDetumbleTests);
    RUN_TEST_GROUP(ADCSPointingTests);
    RUN_TEST_GROUP(ADCSZeroTorqueTests);
}

void setup() {
    Serial.begin(9600);
    while(!Serial);
    UNITY_BEGIN();
    RUN_TEST_GROUP(ADCSControllerTests);
    UNITY_END();
}

void loop() {}