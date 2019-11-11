#include <ControlTask.hpp>
#include "../../src/FCCode/MainControlLoop.hpp"
#include <unity.h>

// Ensure that main control loop initializes properly and
// doesn't segfault upon execution a few times.

void test_task_initialization() {
    StateFieldRegistry registry;
    MainControlLoop fcp(registry);
}

void test_task_execute() {
    StateFieldRegistry registry;
    MainControlLoop fcp(registry);
    for(int i = 0; i < 5; i++) {
        fcp.execute();
    }
}

int test_main_control_loop() {
    UNITY_BEGIN();
    RUN_TEST(test_task_initialization);
    RUN_TEST(test_task_execute);
    return UNITY_END();
}

#ifdef DESKTOP
int main() {
    return test_main_control_loop();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_main_control_loop();
}

void loop() {}
#endif
