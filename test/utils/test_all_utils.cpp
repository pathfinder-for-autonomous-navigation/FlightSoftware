#include <unity.h>
#include "utils_tests.hpp"

int main(int argc, char *argv[]) {
    UNITY_BEGIN();
    test_circular_buffer();
    test_circular_stack();
    UNITY_END();

    return 0;
}