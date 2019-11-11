#include <StateFieldRegistry.hpp>
#include "../../src/FCCode/DownlinkProducer.hpp"
#include <unity.h>

void test_task_initialization() {
    StateFieldRegistry registry;
    DownlinkProducer task(registry);
}

int test_downlink_producer_task() {
    UNITY_BEGIN();
    RUN_TEST(test_task_initialization);
    return UNITY_END();
}

#ifdef DESKTOP
int main() {
    return test_downlink_producer_task();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_downlink_producer_task();
}

void loop() {}
#endif
