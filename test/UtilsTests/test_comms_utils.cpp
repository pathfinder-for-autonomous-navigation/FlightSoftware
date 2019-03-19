#include <unity_fixture.h>
#include <comms_utils.hpp>
#include <array>

TEST_GROUP(CommsUtilsTests);
TEST_SETUP(CommsUtilsTests) {}
TEST_TEAR_DOWN(CommsUtilsTests) {}

TEST(CommsUtilsTests, trim_expand_float) {
    float f = 0.99;
    constexpr size_t bitset_size = 10;
    std::bitset<bitset_size> f_repr;
    Comms::trim_float(f, 0, 10, &f_repr);
    float f_exp = Comms::expand_float(f_repr, 0, 10);

    TEST_ASSERT_FLOAT_WITHIN(f, f_exp, f / pow(2, bitset_size));
}
TEST(CommsUtilsTests, trim_expand_double) {
    double d = 0.99;
    constexpr size_t bitset_size = 10;
    std::bitset<bitset_size> d_repr;
    Comms::trim_double(d, 0, 10, &d_repr);
    double d_exp = Comms::expand_double(d_repr, 0, 10);

    TEST_ASSERT_DOUBLE_WITHIN(d, d_exp, d / pow(2, bitset_size));
}
TEST(CommsUtilsTests, trim_expand_int) {
    int i = 12;
    constexpr size_t bitset_size = 10;
    std::bitset<bitset_size> i_repr;
    Comms::trim_int<bitset_size>(i, 0, 10, &i_repr);
    int i_exp = Comms::expand_int<bitset_size>(i_repr, 0, 10);

    TEST_ASSERT_EQUAL(i, i_exp);
}
TEST(CommsUtilsTests, trim_expand_temperature) {
    int t = 12;
    std::bitset<(size_t) 9> t_repr;
    Comms::trim_temperature(t, &t_repr);
    int t_exp = Comms::expand_temperature(t_repr);

    TEST_ASSERT_EQUAL(t, t_exp);
}

TEST(CommsUtilsTests, trim_expand_vector_float) {}
TEST(CommsUtilsTests, trim_expand_vector_double) {}
TEST(CommsUtilsTests, trim_expand_quaternion) {}
TEST(CommsUtilsTests, trim_expand_gps_time) {}

TEST_GROUP_RUNNER(CommsUtilsTests) {
    RUN_TEST_CASE(CommsUtilsTests, trim_expand_float) {}
    RUN_TEST_CASE(CommsUtilsTests, trim_expand_double) {}
    RUN_TEST_CASE(CommsUtilsTests, trim_expand_int) {}
    RUN_TEST_CASE(CommsUtilsTests, trim_expand_temperature) {}
    RUN_TEST_CASE(CommsUtilsTests, trim_expand_vector_float) {}
    RUN_TEST_CASE(CommsUtilsTests, trim_expand_vector_double) {}
    RUN_TEST_CASE(CommsUtilsTests, trim_expand_quaternion) {}
    RUN_TEST_CASE(CommsUtilsTests, trim_expand_gps_time) {}
}