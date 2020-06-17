#include "fsw_tests.hpp"
#include <unity.h>

int test_fsw_all_1() {
    UNITY_BEGIN();
    RUN_TEST(fsw_test::test_adcs_commander);
    RUN_TEST(fsw_test::test_adcs_box_monitor);
    return UNITY_END();
}

#ifdef COMBINE_TESTS
#ifdef DESKTOP
int main()
{
    return test_fsw_all_1();
}
#else
#include <Arduino.h>
void setup()
{
    delay(2000);
    Serial.begin(9600);
    test_fsw_all_1();
}

void loop() {}
#endif
#endif
