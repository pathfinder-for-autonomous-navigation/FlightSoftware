#include <unity.h>
#include <core_pins.h>
#include <usb_serial.h>
#include <array>
#include "../lib/Drivers/Piksi.hpp"

Devices::Piksi piksi("piksi", Serial4);
int setup_start_time;

void test_piksi_functional() {
    setup_start_time = millis();
    piksi.setup();
    Serial.println("Attempting to get solution...");
    
    std::array<double, 3> pos = {0};
    while (!piksi.process_buffer() || pos[0] == 0) {
        delayMicroseconds(1000);
        piksi.get_pos_ecef(&pos);
    };

    Serial.printf("Setup time: %d ms\n", millis() - setup_start_time);
    Serial.printf("GPS position: %lf,%lf,%lf\n", pos[0], pos[1], pos[2]);
    double pos_mag = sqrt(pos[0]*pos[0] + pos[1]*pos[1] + pos[2]*pos[2]);
    TEST_ASSERT_DOUBLE_WITHIN(1E3, 6.37E6, pos_mag); // We're somewhere on Earth.
    TEST_ASSERT_GREATER_THAN(4, piksi.get_pos_ecef_nsats()); // We need at least 4 satellites to get position.
}

int main(void) {
    delay(5000);
    Serial.begin(9600);
    while (!Serial);
    UNITY_BEGIN();
    RUN_TEST(test_piksi_functional);
    UNITY_END();
    return 0;
}
