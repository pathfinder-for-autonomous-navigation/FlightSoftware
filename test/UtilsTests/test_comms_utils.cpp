#include <unity_test/unity_fixture.h>
#include <comms_utils.hpp>
#include <array>

TEST_GROUP(CommsUtilsTests);
TEST_SETUP(CommsUtilsTests) {}
TEST_TEAR_DOWN(CommsUtilsTests) {}

TEST(CommsUtilsTests, trim_float) {
    float f = 0.5f;
    std::bitset<10> f_repr;
    Comms::trim_float(f, 0, 10, &f_repr);
    float f_exp = Comms::expand_float(f_repr, 0, 10);
    TEST_ASSERT_EQUAL(f, f_exp);
}
TEST(CommsUtilsTests, trim_double) {}
TEST(CommsUtilsTests, trim_vector_float) {}
TEST(CommsUtilsTests, trim_vector_double) {}
TEST(CommsUtilsTests, trim_int) {}
TEST(CommsUtilsTests, trim_temperature) {}
TEST(CommsUtilsTests, trim_quaternion) {}
TEST(CommsUtilsTests, trim_gps_time) {}
TEST(CommsUtilsTests, expand_float) {}
TEST(CommsUtilsTests, expand_double) {}
TEST(CommsUtilsTests, expand_vector_float) {}
TEST(CommsUtilsTests, expand_vector_double) {}
TEST(CommsUtilsTests, expand_int) {}
TEST(CommsUtilsTests, expand_temperature) {}
TEST(CommsUtilsTests, expand_quaternion) {}
TEST(CommsUtilsTests, expand_gps_time) {}

TEST_GROUP_RUNNER(CommsUtilsTests) {
    RUN_TEST_CASE(CommsUtilsTests, trim_float) {}
    RUN_TEST_CASE(CommsUtilsTests, trim_double) {}
    RUN_TEST_CASE(CommsUtilsTests, trim_vector_float) {}
    RUN_TEST_CASE(CommsUtilsTests, trim_vector_double) {}
    RUN_TEST_CASE(CommsUtilsTests, trim_int) {}
    RUN_TEST_CASE(CommsUtilsTests, trim_temperature) {}
    RUN_TEST_CASE(CommsUtilsTests, trim_quaternion) {}
    RUN_TEST_CASE(CommsUtilsTests, trim_gps_time) {}
    RUN_TEST_CASE(CommsUtilsTests, expand_float) {}
    RUN_TEST_CASE(CommsUtilsTests, expand_double) {}
    RUN_TEST_CASE(CommsUtilsTests, expand_vector_float) {}
    RUN_TEST_CASE(CommsUtilsTests, expand_vector_double) {}
    RUN_TEST_CASE(CommsUtilsTests, expand_int) {}
    RUN_TEST_CASE(CommsUtilsTests, expand_temperature) {}
    RUN_TEST_CASE(CommsUtilsTests, expand_quaternion) {}
    RUN_TEST_CASE(CommsUtilsTests, expand_gps_time) {}
}