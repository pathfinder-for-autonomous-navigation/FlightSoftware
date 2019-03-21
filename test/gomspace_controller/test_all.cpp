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

thread_t* gomspace_thread;
TEST_GROUP(GomspaceTests);
TEST_SETUP(GomspaceTests) {
    gomspace_thread = chThdCreateStatic(RTOSTasks::gomspace_controller_workingArea, 
                                    sizeof(RTOSTasks::gomspace_controller_workingArea), 
                                    RTOSTasks::gomspace_thread_priority,
                                    RTOSTasks::gomspace_controller,
                                    NULL);
}
TEST_TEAR_DOWN(GomspaceTests) {
    chThdTerminate(gomspace_thread);
}

TEST_GROUP_RUNNER(GomspaceTests) {
    RUN_TEST_GROUP(GomspaceReadTests);
    RUN_TEST_GROUP(GomspaceCheckTests);
}

void test_setup() {
    RTOSTasks::initialize_rtos_objects();
    chThdSleepMilliseconds(2000);
    UNITY_BEGIN();
    RUN_TEST_GROUP(GomspaceTests);
    UNITY_END();
    chThdExit((msg_t) 0);
}

void setup() {
    Serial.begin(9600);
    chBegin(test_setup);
}

void loop() {}