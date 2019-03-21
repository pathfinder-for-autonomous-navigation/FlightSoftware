#include <unity_fixture.h>
#include <ChRt.h>
#include <rwmutex.hpp>
#include <Arduino.h>
#include "../../src/FCCode/controllers/controllers.hpp"

// Weird bugfix required to fix linker error
extern "C"{
    int _getpid(){ return -1;}
    int _kill(int pid, int sig){ return -1; }
    int _write(){return -1;}
}

thread_t* adcs_thread;
TEST_GROUP(ADCSTests);
TEST_SETUP(ADCSTests) {
    adcs_thread = chThdCreateStatic(RTOSTasks::adcs_controller_workingArea, 
                                    sizeof(RTOSTasks::adcs_controller_workingArea), 
                                    RTOSTasks::adcs_thread_priority,
                                    RTOSTasks::adcs_controller,
                                    NULL);
}
TEST_TEAR_DOWN(ADCSTests) {
    chThdTerminate(adcs_thread);
}

TEST_GROUP_RUNNER(ADCSTests) {
    RUN_TEST_GROUP(ADCSInitializationTests);
    RUN_TEST_GROUP(ADCSDetumbleTests);
    RUN_TEST_GROUP(ADCSPointingTests);
    RUN_TEST_GROUP(ADCSZeroTorqueTests);
}

void test_setup() {
    RTOSTasks::initialize_rtos_objects();
    chThdSleepMilliseconds(2000);
    UNITY_BEGIN();
    RUN_TEST_GROUP(ADCSTests);
    UNITY_END();
    chThdExit((msg_t) 0);
}

void setup() {
    Serial.begin(9600);
    chBegin(test_setup);
}

void loop() {}