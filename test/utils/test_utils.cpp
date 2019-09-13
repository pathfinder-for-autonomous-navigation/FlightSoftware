#include "test_utils.hpp"
#include <math.h>

void test_utils() {
    UNITY_BEGIN();
    test_bit_array();
    test_serializers();
    test_static_buffers();
    UNITY_END();
}

#ifdef DESKTOP
int main(int argc, char *argv[]) {
    test_utils();
    return 0;
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_utils();
}

void loop() {}
#endif
