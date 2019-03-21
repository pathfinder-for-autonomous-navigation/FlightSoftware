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

TEST_GROUP(RWMutexTests);
TEST_SETUP(RWMutexTests) {}
TEST_TEAR_DOWN(RWMutexTests) {}

int w = 2;
rwmutex_t l;
static THD_WORKING_AREA(reader1_wA, 256);
static THD_WORKING_AREA(reader2_wA, 256);
static THD_FUNCTION(reader1, args) {
    rwMtxRLock(&l);
        
    rwMtxRUnlock(&l);
}
static THD_FUNCTION(reader2, args) {
    
}
TEST(RWMutexTests, test_rw_mutex_two_readers) {
    TEST_ASSERT(false);
}

TEST_GROUP_RUNNER(RWMutexTests) {
    RUN_TEST_CASE(RWMutexTests, test_rw_mutex);
}

void setup() {
    delay(2000);
    Serial.begin(9600);
    while(!Serial);
    UNITY_BEGIN();
    RUN_TEST_GROUP(RWMutexTests);
    UNITY_END();
}

void loop() {}