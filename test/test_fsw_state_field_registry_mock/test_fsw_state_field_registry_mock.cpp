#include "../StateFieldRegistryMock.hpp"

#include <unity.h>

void test_valid_initialization() { StateFieldRegistryMock registry; }

int test_state_field_mock() {
    UNITY_BEGIN();
    RUN_TEST(test_valid_initialization);
    return UNITY_END();
}

#ifdef DESKTOP
int main() {
    return test_state_field_mock();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_state_field_mock();
}

void loop() {}
#endif
