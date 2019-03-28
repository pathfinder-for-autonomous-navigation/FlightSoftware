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

thread_t* propulsion_thread;
TEST_GROUP(PropulsionTests);
TEST_SETUP(PropulsionTests) {
    propulsion_thread = chThdCreateStatic(RTOSTasks::propulsion_controller_workingArea, 
                                    sizeof(RTOSTasks::propulsion_controller_workingArea), 
                                    RTOSTasks::propulsion_thread_priority,
                                    RTOSTasks::propulsion_controller,
                                    NULL);
}
TEST_TEAR_DOWN(PropulsionTests) {
    chThdTerminate(propulsion_thread);
}

TEST_GROUP_RUNNER(PropulsionTests) {
    RUN_TEST_GROUP(PropulsionDisabledTests);
    RUN_TEST_GROUP(PropulsionIdleTests);
    RUN_TEST_GROUP(PropulsionAwaitingPressurizationTests);
    RUN_TEST_GROUP(PropulsionPressurizingTests);
    RUN_TEST_GROUP(PropulsionVentingTests);
    RUN_TEST_GROUP(PropulsionFiringTests);
}

void test_setup() {
    RTOSTasks::initialize_rtos_objects();
    chThdSleepMilliseconds(2000);
    UNITY_BEGIN();
    RUN_TEST_GROUP(PropulsionTests);
    UNITY_END();
    chThdExit((msg_t) 0);
}

void setup() {
    Serial.begin(9600);
    chBegin(test_setup);
}

void loop() {}