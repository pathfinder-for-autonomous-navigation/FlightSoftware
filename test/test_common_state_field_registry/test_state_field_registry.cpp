#include <common/StateFieldRegistry.hpp>

#include <unity.h>

void test_foo() {
    StateFieldRegistry registry;
}

void test_state_field_registry() {
    UNITY_BEGIN();
    RUN_TEST(test_foo);
    UNITY_END();
}

#ifdef DESKTOP
int main(int argc, char *argv[]) {
    test_state_field_registry();
    return 0;
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_state_field_registry();
}

void loop() {}
#endif
