#include <unity.h>
#include <comms_utils.hpp>
#include <array>
#include "utils_tests.hpp"

static void comms_utils_trim_expand_float(void) {
    float f = 0.99;
    constexpr size_t bitset_size = 10;
    std::bitset<bitset_size> f_repr;
    Comms::trim_float(f, 0, 10, &f_repr);
    float f_exp = Comms::expand_float(f_repr, 0, 10);

    TEST_ASSERT_FLOAT_WITHIN(f, f_exp, f / pow(2, bitset_size));
}
static void comms_utils_trim_expand_double(void) {
    double d = 0.99;
    constexpr size_t bitset_size = 10;
    std::bitset<bitset_size> d_repr;
    Comms::trim_double(d, 0, 10, &d_repr);
    double d_exp = Comms::expand_double(d_repr, 0, 10);

    TEST_ASSERT_DOUBLE_WITHIN(d, d_exp, d / pow(2, bitset_size));
}
static void comms_utils_trim_expand_int(void) {
    int i = 9;
    constexpr size_t bitset_size = 10;
    std::bitset<bitset_size> i_repr;
    Comms::trim_int<bitset_size>(i, 0, 10, &i_repr);
    int i_exp = Comms::expand_int<bitset_size>(i_repr, 0, 10);

    TEST_ASSERT_EQUAL(i, i_exp);
}
static void comms_utils_trim_expand_temperature(void) {
    int t = 12;
    std::bitset<(size_t) 9> t_repr;
    Comms::trim_temperature(t, &t_repr);
    int t_exp = Comms::expand_temperature(t_repr);

    TEST_ASSERT_EQUAL(t, t_exp);
}

static void comms_utils_trim_expand_vector_float(void) {
    std::array<float, 3> vec = {12, 12, 10};
    std::bitset<38> vec_repr;
    Comms::trim_vector(vec, -20, 20, &vec_repr);
    std::array<float, 3> vec_exp;
    Comms::expand_vector(vec_repr, -20, 20, &vec_exp);
    
    TEST_ASSERT_FLOAT_WITHIN(vec[0], vec_exp[0], 1);
    TEST_ASSERT_FLOAT_WITHIN(vec[1], vec_exp[1], 1);
    TEST_ASSERT_FLOAT_WITHIN(vec[2], vec_exp[2], 1);
}

static void comms_utils_trim_expand_vector_double(void) {
    std::array<double, 3> vec = {12, 12, 10};
    std::bitset<40> vec_repr;
    Comms::trim_vector(vec, -20, 20, &vec_repr);
    std::array<double, 3> vec_exp;
    Comms::expand_vector(vec_repr, -20, 20, &vec_exp);

    TEST_ASSERT_FLOAT_WITHIN(vec[0], vec_exp[0], 1);
    TEST_ASSERT_FLOAT_WITHIN(vec[1], vec_exp[1], 1);
    TEST_ASSERT_FLOAT_WITHIN(vec[2], vec_exp[2], 1);
}

static void comms_utils_trim_expand_quaternion(void) {
    std::array<float, 4> quat = {0.5, 0.5, 0.5, 0.8};
    std::bitset<29> quat_repr;
    Comms::trim_quaternion(quat, &quat_repr);
    std::array<float, 4> quat_exp;
    Comms::expand_quaternion(quat_repr, &quat_exp);

    TEST_ASSERT_FLOAT_WITHIN(quat[0], quat_exp[0], 1);
    TEST_ASSERT_FLOAT_WITHIN(quat[1], quat_exp[1], 1);
    TEST_ASSERT_FLOAT_WITHIN(quat[2], quat_exp[2], 1);
    TEST_ASSERT_FLOAT_WITHIN(quat[3], quat_exp[3], 1);
}

static void comms_utils_trim_expand_gps_time(void) {
    TEST_ASSERT(false);
}

static void comms_utils_trim_expand_message(void) {
    TEST_ASSERT(false);
}

void test_comms_utils() {
    RUN_TEST(comms_utils_trim_expand_float);
    RUN_TEST(comms_utils_trim_expand_double);
    RUN_TEST(comms_utils_trim_expand_int);
    RUN_TEST(comms_utils_trim_expand_temperature);
    RUN_TEST(comms_utils_trim_expand_vector_float);
    RUN_TEST(comms_utils_trim_expand_vector_double);
    RUN_TEST(comms_utils_trim_expand_quaternion);
    RUN_TEST(comms_utils_trim_expand_gps_time);
    RUN_TEST(comms_utils_trim_expand_message);
}