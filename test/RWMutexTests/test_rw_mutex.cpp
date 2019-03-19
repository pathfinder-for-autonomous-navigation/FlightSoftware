#include <unity_fixture.h>
#include <Arduino.h>

// Weird bugfix required to fix linker error
extern "C"{
    int _getpid(){ return -1;}
    int _kill(int pid, int sig){ return -1; }
    int _write(){return -1;}
}

TEST_GROUP(RWMutexTests);
TEST_SETUP(RWMutexTests) {}
TEST_TEAR_DOWN(RWMutexTests) {}

TEST(RWMutexTests, test_rw_mutex) {
    TEST_ASSERT(false);
}

TEST_GROUP_RUNNER(RWMutexTests) {
    RUN_TEST_CASE(RWMutexTests, test_rw_mutex);
}

void setup() {
    Serial.begin(9600);
    while(!Serial);
    UNITY_BEGIN();
    RUN_TEST_GROUP(RWMutexTests);
    UNITY_END();
}

void loop() {}