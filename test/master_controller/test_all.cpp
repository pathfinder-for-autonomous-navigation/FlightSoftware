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

thread_t* master_thread;
TEST_GROUP(MasterTests);
TEST_SETUP(MasterTests) {
    master_thread = chThdCreateStatic(RTOSTasks::master_controller_workingArea, 
                                    sizeof(RTOSTasks::master_controller_workingArea), 
                                    RTOSTasks::master_thread_priority,
                                    RTOSTasks::master_controller,
                                    NULL);
}
TEST_TEAR_DOWN(MasterTests) {
    chThdTerminate(master_thread);
}

TEST_GROUP_RUNNER(MasterTests) {
    RUN_TEST_GROUP(MasterInitTests);
    RUN_TEST_GROUP(MasterSafeHoldTests);
    RUN_TEST_GROUP(MasterInitializationHoldTests);
    RUN_TEST_GROUP(MasterDetumbleTests);
    RUN_TEST_GROUP(MasterStandbyTests);
    RUN_TEST_GROUP(MasterLeaderCloseApproachTests);
    RUN_TEST_GROUP(MasterSpacejunkTests);
    RUN_TEST_GROUP(MasterFollowerTests);
    RUN_TEST_GROUP(MasterFollowerCloseApproachTests);
    RUN_TEST_GROUP(MasterPairedTests);
    RUN_TEST_GROUP(MasterDockingTests);
    RUN_TEST_GROUP(MasterDockedTests);
}

void test_setup() {
    RTOSTasks::initialize_rtos_objects();
    chThdSleepMilliseconds(2000);
    UNITY_BEGIN();
    RUN_TEST_GROUP(MasterTests);
    UNITY_END();
    chThdExit((msg_t) 0);
}

void setup() {
    Serial.begin(9600);
    chBegin(test_setup);
}

void loop() {}