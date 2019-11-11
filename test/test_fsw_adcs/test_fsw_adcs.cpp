#include <unity.h>

void test_adcs_attitude_computer() {}
void test_adcs_attitude_estimator() {}
void test_adcs_box_controller() {}
void test_adcs_box_monitor() {}

int test_adcs_all() {
    UNITY_BEGIN();
    RUN_TEST(test_adcs_attitude_computer);
    RUN_TEST(test_adcs_attitude_estimator);
    RUN_TEST(test_adcs_box_controller);
    RUN_TEST(test_adcs_box_monitor);
    return UNITY_END();
}

#ifdef DESKTOP
int main() {
    return test_adcs_all();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_adcs_all();
}

void loop() {}
#endif
