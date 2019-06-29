#include "utils_tests.hpp"
#include <iostream>
#include <Serializer.hpp>

/**
 * @brief Verify that initialization works correctly.
 */
void test_serializerbase_init_ok() {
    SerializerBase<bool> serializer;
    // Should not be initialized if not initialized
    TEST_ASSERT_FALSE(serializer.is_initialized());
    // Should be initialized if initialized
    serializer.init(false, true);
    TEST_ASSERT(serializer.is_initialized());

    // Should fail initialization if maximum is less than minimum
    Serializer<unsigned int, unsigned int, 10> serializer2;
    TEST_ASSERT_FALSE(serializer2.init(10, 2));
    TEST_ASSERT_FALSE(serializer2.is_initialized());
}

/**
 * @brief Verify that the boolean serializer properly encapsulates
 * a boolean into a bitset.
 */
void test_bool_serializer() {
    Serializer<bool, bool, SerializerConstants::bool_sz> serializer;
    serializer.init(false, true);

    std::bitset<SerializerConstants::bool_sz> dest;
    serializer.serialize(true, &dest);
    TEST_ASSERT(dest[0]);
    serializer.serialize(false, &dest);
    TEST_ASSERT_FALSE(dest[0]);
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
    /**
     * Test serializer with a number of bits that is more than what's needed
     * to represent a value in the range. Verify that all values can be
     * represented correctly.
     */
    Serializer<unsigned int, unsigned int, 10> serializer; // 2^10 = 1024, which is greater than 20
    serializer.init(0, 20);
    
    // Dummy variables needed for loop
    std::bitset<10> bitset;
    unsigned int result;
    for (unsigned int i = 0; i <= 20; i++) {
        serializer.serialize(i, &bitset);
        serializer.deserialize(bitset, &result);
        TEST_ASSERT_EQUAL(i, result);
    }
    // Test the same thing with a serializer that doesn't begin at zero.
    serializer.init(5, 25);
    for (unsigned int i = 5; i <= 25; i++)
    {
        serializer.serialize(i, &bitset);
        serializer.deserialize(bitset, &result);
        TEST_ASSERT_EQUAL(i, result);
    }

    /**
     * Test serializer with a number of bits that is less than what's needed
     * to represent a value in the range. Verify that all values can be
     * represented correctly.
     */
    Serializer<unsigned int, unsigned int, 4> serializer2; // 2^4 = 16, which is less than 20
    serializer2.init(0, 20);

    // Dummy variables needed for loop
    std::bitset<4> bitset2;
    unsigned int result2;
    for (unsigned int i = 0; i <= 20; i++)
    {
        serializer2.serialize(i, &bitset2);
        serializer2.deserialize(bitset2, &result2);
        TEST_ASSERT_EQUAL(i, result);
    }
    // Test the same thing with a serializer that doesn't begin at zero.
    serializer2.init(5, 25);
    for (unsigned int i = 5; i <= 25; i++)
    {
        serializer2.serialize(i, &bitset2);
        serializer2.deserialize(bitset2, &result2);
        TEST_ASSERT_EQUAL(i, result);
    }
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
void test_sint_serializer() { /* TODO */ }

/**
 * @brief Verify that the float serializer properly
 * encapsulates float values of various sizes.
 * 
 * Success criteria: same as signed int serializer. Comparisons
 * for accuracy, however, cannot be an equality--they must be
 * based on resolution thresholds.
 */
void test_float_serializer() { /* TODO */ }

/**
 * @brief Verify that the double serializer properly
 * encapsulates double values of various sizes.
 * 
 * Success criteria: same as float.
 */
void test_double_serializer() { /* TODO */ }

/**
 * @brief Verify that the float vector serializer properly
 * encapsulates float vectors of various sizes.
 */
void test_f_vec_serializer() { /* TODO */ }

/**
 * @brief Verify that the double vector serializer properly
 * encapsulates double vectors of various sizes.
 */
void test_d_vec_serializer() { /* TODO */ }

/**
 * @brief Verify that the float quaternion serializer properly
 * encapsulates float quaternions of various sizes.
 */
void test_f_quat_serializer() { /* TODO */ }

/**
 * @brief Verify that the double quaternion serializer properly
 * encapsulates double quaternions of various sizes.
 */
void test_d_quat_serializer() { /* TODO */ }

/**
 * @brief Verify that the GPS time serializer properly
 * encapsulates various values of GPS time.
 */
void test_gpstime_serializer() { /* TODO */ }

/**
 * @brief Verify that the temperature serializer properly
 * encapsulates various temperatures.
 */
void test_temperature_serializer() { /* TODO */ }

void test_serializers() {
    RUN_TEST(test_serializerbase_init_ok);

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
    RUN_TEST(test_temperature_serializer);
}