#include "utils_tests.hpp"

int main(int argc, char *argv[]) {
    UNITY_BEGIN();
    test_static_buffers();
    test_serializers();
    UNITY_END();

    return 0;
}