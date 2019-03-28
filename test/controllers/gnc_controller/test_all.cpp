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

thread_t* gnc_thread;
TEST_GROUP(GNCTests);
TEST_SETUP(GNCTests) {
    gnc_thread = chThdCreateStatic(RTOSTasks::gnc_controller_workingArea, 
                                    sizeof(RTOSTasks::gnc_controller_workingArea), 
                                    RTOSTasks::gnc_thread_priority,
                                    RTOSTasks::gnc_controller,
                                    NULL);
}
TEST_TEAR_DOWN(GNCTests) {
    chThdTerminate(gnc_thread);
}

TEST(GNCTests, test_firing_bounds) {}
TEST(GNCTests, test_propagation) {}
TEST(GNCTests, test_eci_to_ecef_quaternion_update) {}
TEST(GNCTests, test_eci_to_lvlh_quaternion_update) {}

TEST_GROUP_RUNNER(GNCTests) {
    RUN_TEST_CASE(GNCTests, test_firing_bounds);
    RUN_TEST_CASE(GNCTests, test_propagation);
    RUN_TEST_CASE(GNCTests, test_eci_to_ecef_quaternion_update);
    RUN_TEST_CASE(GNCTests, test_eci_to_lvlh_quaternion_update);
}

void test_setup() {
    RTOSTasks::initialize_rtos_objects();
    chThdSleepMilliseconds(2000);
    UNITY_BEGIN();
    RUN_TEST_GROUP(GNCTests);
    UNITY_END();
    chThdExit((msg_t) 0);
}

void setup() {
    Serial.begin(9600);
    chBegin(test_setup);
}

void loop() {}