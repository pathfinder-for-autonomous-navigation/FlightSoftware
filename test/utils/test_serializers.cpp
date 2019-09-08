#include <Serializer.hpp>
#include <iostream>
#include "utils_tests.hpp"

/**
 * @brief Verify that the boolean serializer properly encapsulates
 * a boolean into a bitset.
 */
void test_bool_serializer() {
    Serializer<bool> serializer;
    auto dest_ptr = std::make_shared<bool>();

    // Normal serialize and deserialize
    serializer.serialize(true);
    serializer.deserialize(dest_ptr);
    TEST_ASSERT(*dest_ptr);
    serializer.serialize(false);
    serializer.deserialize(dest_ptr);
    TEST_ASSERT_FALSE(*dest_ptr);

    // String-based deserialize
    serializer.deserialize("true", dest_ptr);
    TEST_ASSERT(*dest_ptr);
    serializer.deserialize("false", dest_ptr);
    TEST_ASSERT_FALSE(*dest_ptr);

    // Printing
    TEST_ASSERT_EQUAL_STRING(serializer.print(true), "true");
    TEST_ASSERT_EQUAL_STRING(serializer.print(false), "false");
    TEST_ASSERT_EQUAL(serializer.strlen, 5);
    
    // Bit array-related getters
    const bit_array& arr = serializer.get_bit_array();
    TEST_ASSERT_EQUAL(arr.size(), 1);
    TEST_ASSERT_EQUAL(serializer.bitsize(), 1);
    serializer.serialize(true);
    TEST_ASSERT_EQUAL(arr[0], true);
    serializer.serialize(false);
    TEST_ASSERT_EQUAL(arr[0], false);

    // Bit array setter
    bit_array val(1);
    val[0] = true;
    serializer.set_bit_array(val);
    serializer.deserialize(dest_ptr);
    TEST_ASSERT(*dest_ptr);
    val[0] = false;
    serializer.set_bit_array(val);
    serializer.deserialize(dest_ptr);
    TEST_ASSERT_FALSE(*dest_ptr);
}

/**
 * @brief Verify that the unsigned int serializer properly
 * encapsulates integers of various sizes.
 *
 * Success criteria:
 * - Bitsets with more bits that needed to fully contain the range
 *   do not lose resolution.
 * - Bitsets with fewer bits than needed to fully contain the range
 *   of the serializer lose resolution in a predictable way.
 * - Ranges that start with zero work just as well as ranges that don't
 *   start at zero.
 */
void test_uint_serializer() {
    TEST_ASSERT(false);
}

/**
 * @brief Verify that the signed int serializer properly
 * encapsulates integers of various sizes.
 *
 * Success criteria:
 * - Same as unsigned int critera, with the addition of
 * - Ranges that start at negative values work just as well
 *   as other range starts.
 */
void test_sint_serializer() {
    TEST_ASSERT(false);
}

/**
 * @brief Verify that the float serializer properly
 * encapsulates float values of various sizes.
 *
 * Success criteria: same as signed int serializer. Comparisons
 * for accuracy, however, cannot be an equality--they must be
 * based on resolution thresholds.
 */
void test_float_serializer() {
    TEST_ASSERT(false);
}

/**
 * @brief Verify that the double serializer properly
 * encapsulates double values of various sizes.
 *
 * Success criteria: same as float.
 */
void test_double_serializer() {
    TEST_ASSERT(false);
}

/**
 * @brief Verify that the float vector serializer properly
 * encapsulates float vectors of various sizes.
 */
void test_f_vec_serializer() {
    TEST_ASSERT(false);
}

/**
 * @brief Verify that the double vector serializer properly
 * encapsulates double vectors of various sizes.
 */
void test_d_vec_serializer() {
    TEST_ASSERT(false);
}

/**
 * @brief Verify that the float quaternion serializer properly
 * encapsulates float quaternions of various sizes.
 */
void test_f_quat_serializer() {
    TEST_ASSERT(false);
}

/**
 * @brief Verify that the double quaternion serializer properly
 * encapsulates double quaternions of various sizes.
 */
void test_d_quat_serializer() {
    TEST_ASSERT(false);
}

/**
 * @brief Verify that the GPS time serializer properly
 * encapsulates various values of GPS time.
 */
void test_gpstime_serializer() {
    TEST_ASSERT(false);
}

/**
 * @brief Verify that the temperature serializer properly
 * encapsulates various temperatures.
 */
void test_temperature_serializer() {
    TEST_ASSERT(false);
}

void test_serializers() {
    RUN_TEST(test_bool_serializer);
    RUN_TEST(test_uint_serializer);
    RUN_TEST(test_sint_serializer);
    RUN_TEST(test_float_serializer);
    RUN_TEST(test_double_serializer);
    RUN_TEST(test_f_vec_serializer);
    RUN_TEST(test_d_vec_serializer);
    RUN_TEST(test_f_quat_serializer);
    RUN_TEST(test_d_quat_serializer);
    RUN_TEST(test_gpstime_serializer);
}