#include <StateFieldRegistry.hpp>
#include "DummyControlTask.hpp"
#include <unity.h>

void test_foo() {
    StateFieldRegistry registry;
    DummyControlTask task(registry);
    TEST_ASSERT_EQUAL(2, task.x);
}

int test_control_task() {
    UNITY_BEGIN();
    RUN_TEST(test_foo);
    return UNITY_END();
}

#ifdef DESKTOP
int main() {
    return test_control_task();
}
#else 
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_control_task();
}

void loop() {}
#endif
