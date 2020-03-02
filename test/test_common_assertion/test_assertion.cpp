#include <common/assertion.hpp>

#include <unity.h>

void test_assertion_behavior() {
    // A correct assertion does not cause any failures
    pan_assert<std::logic_error>(true, "No failures.");

    // An incorrect assertion causes a failure.
    try {
        pan_assert<std::logic_error>(false, "This should cause an exception throw.");
        TEST_FAIL_MESSAGE("Should've thrown the exception by this line.");
    }
    catch(std::exception& e) {
        // The exception was thrown.
        TEST_PASS();
    }
}

int test_assertion() {
    UNITY_BEGIN();
    RUN_TEST(test_assertion_behavior);
    return UNITY_END();
}

#ifdef DESKTOP
int main() {
    return test_assertion();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_assertion();
}

void loop() {}
#endif
