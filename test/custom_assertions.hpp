#ifndef CUSTOM_ASSERTIONS_HPP_
#define CUSTOM_ASSERTIONS_HPP_

#include <unity.h>
#undef isnan
#undef isinf

#define PAN_TEST_ASSERT_EQUAL_FLOAT_ARR(expected, actual, delta, n) { \
    char err_str[25]; \
    for(size_t i = 0; i < n; i++) { \
        sprintf(err_str, "Fail on element %d", (int) i); \
        TEST_ASSERT_FLOAT_WITHIN_MESSAGE(delta, expected[i], actual[i], err_str); \
    } \
}

#define PAN_TEST_ASSERT_EQUAL_DOUBLE_ARR(expected, actual, delta, n) { \
    char err_str[25]; \
    for(size_t i = 0; i < n; i++) { \
        sprintf(err_str, "Fail on element %d", (int) i); \
        TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(delta, expected[i], actual[i], err_str); \
    } \
}

#define PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(expected, actual, delta)   PAN_TEST_ASSERT_EQUAL_FLOAT_ARR(expected, actual, delta, 3)
#define PAN_TEST_ASSERT_EQUAL_FLOAT_QUAT(expected, actual, delta)  PAN_TEST_ASSERT_EQUAL_FLOAT_ARR(expected, actual, delta, 4)
#define PAN_TEST_ASSERT_EQUAL_DOUBLE_VEC(expected, actual, delta)  PAN_TEST_ASSERT_EQUAL_DOUBLE_ARR(expected, actual, delta, 3)
#define PAN_TEST_ASSERT_EQUAL_DOUBLE_QUAT(expected, actual, delta) PAN_TEST_ASSERT_EQUAL_DOUBLE_ARR(expected, actual, delta, 4)

#define PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(expected, actual, delta) { \
    char err_str[25]; \
    for(size_t i = 0; i < 3; i++) { \
        sprintf(err_str, "Fail on element %d", (int) i); \
        TEST_ASSERT_FLOAT_WITHIN_MESSAGE(delta, expected(i), actual(i), err_str); \
    } \
}

#define PAN_TEST_ASSERT_EQUAL_DOUBLE_LIN_VEC(expected, actual, delta) { \
    char err_str[25]; \
    for(size_t i = 0; i < 3; i++) { \
        sprintf(err_str, "Fail on element %d", (int) i); \
        TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(delta, expected(i), actual(i), err_str); \
    } \
}

#define PAN_TEST_ASSERT_NOT_NAN_DOUBLE_LIN_VEC(actual) { \
    char err_str[25]; \
    for(size_t i = 0; i < 3; i++) { \
        sprintf(err_str, "Fail on element %d", (int) i); \
        TEST_ASSERT_DOUBLE_IS_NOT_NAN_MESSAGE(actual(i), err_str); \
    } \
}

#endif
