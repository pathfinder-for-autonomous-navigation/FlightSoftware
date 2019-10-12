#include "../StateFieldRegistryMock.hpp"
#include <StateField.hpp>
#include <Serializer.hpp>
#include "../src/FCCode/MissionManager.hpp"
#include "../src/FCCode/mission_mode_t.enum"

#include <unity.h>

void test_valid_initialization() {
    StateFieldRegistryMock registry;

    registry.create_writable_field<unsigned int>("adcs.mode", 0, 10, 4);
    registry.create_writable_field<f_quat_t>("adcs.cmd_attitude");
    registry.create_readable_field<float>("adcs.ang_rate", 0, 10, 4);
    registry.create_writable_field<float>("adcs.min_stable_ang_rate", 0, 10, 4);

    MissionManager mission_manager(registry);

    //mission_manager.execute();
}

int test_mission_manager() {
    UNITY_BEGIN();
    RUN_TEST(test_valid_initialization);
    return UNITY_END();
}

#ifdef DESKTOP
int main() {
    return test_mission_manager();
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
