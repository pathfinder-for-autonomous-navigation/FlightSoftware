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

thread_t* quake_thread;
TEST_GROUP(QuakeTests);
TEST_SETUP(QuakeTests) {
    quake_thread = chThdCreateStatic(RTOSTasks::quake_controller_workingArea, 
                                    sizeof(RTOSTasks::quake_controller_workingArea), 
                                    RTOSTasks::quake_thread_priority,
                                    RTOSTasks::quake_controller,
                                    NULL);
}
TEST_TEAR_DOWN(QuakeTests) {
    chThdTerminate(quake_thread);
}

TEST_GROUP_RUNNER(QuakeTests) {
    RUN_TEST_GROUP(QuakeWaitingTests);
    RUN_TEST_GROUP(QuakeTransceivingTests);
}

void test_setup() {
    RTOSTasks::initialize_rtos_objects();
    chThdSleepMilliseconds(2000);
    UNITY_BEGIN();
    RUN_TEST_GROUP(QuakeTests);
    UNITY_END();
    chThdExit((msg_t) 0);
}

void setup() {
    Serial.begin(9600);
    chBegin(test_setup);
}

void loop() {}