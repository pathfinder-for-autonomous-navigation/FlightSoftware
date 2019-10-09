#include <StateFieldRegistry.hpp>
#include <StateField.hpp>
#include "../src/FCCode/MissionManager.hpp"
#include "../src/FCCode/mission_mode_t.enum"

#include <unity.h>

StateFieldRegistry registry;
MissionManager mission_manager(registry);

void test_foo() {
    mission_manager.execute();
}

void test_mission_manager() {
    UNITY_BEGIN();
    RUN_TEST(test_foo);
    UNITY_END();
}

#ifdef DESKTOP
int main(int argc, char *argv[]) {
    test_mission_manager();
    return 0;
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_mission_manager();
}

void loop() {}
#endif
