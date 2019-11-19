#include "../../src/GroundCode/DownlinkParser.hpp"
#include <unity.h>

void test_task_initialization() {
    StateFieldRegistryMock reg;
    reg.create_readable_field<gps_time_t>("foo1");
    reg.create_readable_field<gps_time_t>("foo2");
    DownlinkParser dp(reg);
}

void test_task_execute() {

}

int test_downlink_parser() {
    UNITY_BEGIN();
    RUN_TEST(test_task_initialization);
    RUN_TEST(test_task_execute);
    return UNITY_END();
}

#ifdef DESKTOP
int main() {
    return test_downlink_parser();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_downlink_parser();
}

void loop() {}
#endif
