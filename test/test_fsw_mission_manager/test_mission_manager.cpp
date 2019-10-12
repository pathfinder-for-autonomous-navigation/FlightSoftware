#include <StateFieldRegistry.hpp>
#include <StateField.hpp>
#include <Serializer.hpp>
#include "../src/FCCode/MissionManager.hpp"
#include "../src/FCCode/mission_mode_t.enum"

#include <unity.h>

void test_foo() {
    StateFieldRegistry registry;

    Serializer<unsigned int> adcs_mode_serializer(0, 10, 4);
    WritableStateField<unsigned int> adcs_mode_f("adcs.mode", adcs_mode_serializer);
    std::shared_ptr<WritableStateField<unsigned int>> adcs_mode_f_ptr(
        std::shared_ptr<WritableStateField<unsigned int>>{}, &adcs_mode_f);
    registry.add_writable(adcs_mode_f_ptr);

    Serializer<f_quat_t> adcs_cmd_attitude_serializer;
    WritableStateField<f_quat_t> adcs_cmd_attitude_f("adcs.cmd_attitude", adcs_cmd_attitude_serializer);
    std::shared_ptr<WritableStateField<f_quat_t>> adcs_cmd_attitude_f_ptr(
        std::shared_ptr<WritableStateField<f_quat_t>>{}, &adcs_cmd_attitude_f);
    registry.add_writable(adcs_cmd_attitude_f_ptr);

    MissionManager mission_manager(registry);

    mission_manager.execute();
}

int test_mission_manager() {
    UNITY_BEGIN();
    RUN_TEST(test_foo);
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
