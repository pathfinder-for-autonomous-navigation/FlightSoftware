//
// Created by athena on 9/27/20.
//
#include "../StateFieldRegistryMock.hpp"
#include "../custom_assertions.hpp"

#include <fsw/FCCode/AttitudeController.h>
#include <fsw/FCCode/adcs_state_t.enum>

#include "../custom_assertions.hpp"
#include <gnc/constants.hpp>

class TestFixture {
public:
    StateFieldRegistryMock registry;

    std::unique_ptr<AttitudeController> ac;
};

void test_initialization()
{
    TestFixture tf;
}

int test_attitude_controller() {
    UNITY_BEGIN();
    RUN_TEST(test_initialization);
    return UNITY_END();
}

#ifdef DESKTOP
int main() {
    return test_attitude_controller();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_attitude_computer();
}

void loop() {}
#endif
