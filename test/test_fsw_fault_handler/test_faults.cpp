#include "test_fault_handlers.hpp"
#include "cartesian_product.hpp"

void test_faults() {
    RUN_TEST(test_cartesian_product);
    RUN_TEST(test_fault_machine_mock);
    RUN_TEST(test_super_simple_fault_handlers);
    RUN_TEST(test_main_fault_handler);

    // TODO
    // RUN_TEST(test_prop_overpressure_fault_handler);
    // RUN_TEST(test_quake_fault_handler);
    // RUN_TEST(test_piksi_fault_handler);
    // RUN_TEST(test_fault_handling_end_to_end);
}

#ifdef DESKTOP
int main() {
    UNITY_BEGIN();
    test_faults();
    return UNITY_END();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    UNITY_BEGIN();
    test_faults();
    UNITY_END();
}

void loop() {}
#endif
