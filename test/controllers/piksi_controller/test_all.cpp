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

thread_t* piksi_thread;
TEST_GROUP(PiksiTests);
TEST_SETUP(PiksiTests) {
    piksi_thread = chThdCreateStatic(RTOSTasks::piksi_controller_workingArea, 
                                    sizeof(RTOSTasks::piksi_controller_workingArea), 
                                    RTOSTasks::piksi_thread_priority,
                                    RTOSTasks::piksi_controller,
                                    NULL);
}
TEST_TEAR_DOWN(PiksiTests) {
    chThdTerminate(piksi_thread);
}

TEST(PiksiTests, test_1) {}

TEST_GROUP_RUNNER(PiksiTests) {
    RUN_TEST_CASE(PiksiReadTests, test_1);
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