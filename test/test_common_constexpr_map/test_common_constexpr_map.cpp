#include "../custom_assertions.hpp"
#include <common/ConstexprMap.hpp>

#include <cstring>

int test_constexpr_map() {
    UNITY_BEGIN();

    constexpr ConstexprMap<int, char const *, 5> map {{{
        {3, "three"}, {2, "two"}, {0, "zero"}, {1, "one"}, {4, "four"}
    }}};
    TEST_ASSERT_EQUAL(0, strcmp(map[0], "zero"));
    TEST_ASSERT_EQUAL(0, strcmp(map[1], "one"));
    TEST_ASSERT_EQUAL(0, strcmp(map[2], "two"));
    TEST_ASSERT_EQUAL(0, strcmp(map[3], "three"));
    TEST_ASSERT_EQUAL(0, strcmp(map[4], "four"));

    return UNITY_END();
}

#ifdef DESKTOP
int main(int argc, char *argv[]) {
    return test_constexpr_map();
}
#else
#include <Arduino.h>
void setup() {
    delay(10000);
    Serial.begin(9600);
    test_constexpr_map();
}

void loop() {}
#endif
