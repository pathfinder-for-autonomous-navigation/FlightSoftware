#include <unity.h>
#include <Serializer.hpp>
#include <iostream>
#include <memory>
#include <sstream>

// ============================================================================================= //
//                                      Helper methods                                           //
// ============================================================================================= //

/**
 * @brief Helper methods to test serialization and deserialization.
 *
 * @tparam T Type of serializer being tested
 * @param s Pointer to serializdr object
 * @param val Value to serialize
 * @param output Value to expect when deserializing val (fixed-point compression is lossy
 * compression, so some values are not equal to their output.)
 */
template <typename T>
void test_value(std::shared_ptr<Serializer<T>>& s, const T val, const T output) {
    auto val_ptr = std::make_shared<T>();
    s->serialize(val);
    s->deserialize(val_ptr);
    TEST_ASSERT_EQUAL(output, *val_ptr);
}

/**
 * @brief Helper method to test serialization and deserialization specifically for floats and
 * doubles.
 */
template <typename T>
void test_value_float_or_double(std::shared_ptr<Serializer<T>>& s, const T val, const T output,
                                const T threshold = 0) {
    auto val_ptr = std::make_shared<T>();
    s->serialize(val);
    s->deserialize(val_ptr);

    if (std::is_same<T, float>::value) {
        TEST_ASSERT_FLOAT_WITHIN(threshold, output, *val_ptr);
    } else {
        TEST_ASSERT_DOUBLE_WITHIN(threshold, output, *val_ptr);
    }
}

// ============================================================================================= //
//                              Begin actual test cases                                          //
// ============================================================================================= //

/**
 * @brief Verify that the boolean serializer properly encapsulates a boolean into a bitset.
 */
void test_bool_serializer() {
    auto serializer = std::make_shared<Serializer<bool>>();
    auto dest_ptr = std::make_shared<bool>();

    // Normal serialize and deserialize
    test_value(serializer, true, true);
    test_value(serializer, false, false);

    // String-based deserialize
    serializer->deserialize("true", dest_ptr);
    TEST_ASSERT(*dest_ptr);
    serializer->deserialize("false", dest_ptr);
    TEST_ASSERT_FALSE(*dest_ptr);

    // Printing
    TEST_ASSERT_EQUAL_STRING(serializer->print(true), "true");
    TEST_ASSERT_EQUAL_STRING(serializer->print(false), "false");
    TEST_ASSERT_EQUAL(serializer->strlen, 6);

    // Bit array-related getters
    const bit_array& arr = serializer->get_bit_array();
    TEST_ASSERT_EQUAL(arr.size(), 1);
    TEST_ASSERT_EQUAL(serializer->bitsize(), 1);
    serializer->serialize(true);
    TEST_ASSERT_EQUAL(arr[0], true);
    serializer->serialize(false);
    TEST_ASSERT_EQUAL(arr[0], false);

    // Bit array setter
    bit_array val(1);
    val[0] = true;
    serializer->set_bit_array(val);
    serializer->deserialize(dest_ptr);
    TEST_ASSERT(*dest_ptr);
    val[0] = false;
    serializer->set_bit_array(val);
    serializer->deserialize(dest_ptr);
    TEST_ASSERT_FALSE(*dest_ptr);
}

/**
 * @brief Template function to test common behavior between signed and unsigned int serializers.
 *
 * @tparam T
 */
template <typename T>
void test_int_serializer() {
    std::shared_ptr<Serializer<T>> serializer;

    /** Test edge-case initializations **/
    serializer.reset(new Serializer<T>(0, 0, 0));
    test_value<T>(serializer, 0, 0);
    test_value<T>(serializer, 1, 0);
    serializer.reset(new Serializer<T>(0, 1, 0));
    test_value<T>(serializer, 0, 0);
    test_value<T>(serializer, 1, 0);
    serializer.reset(new Serializer<T>(0, 1, 1));
    test_value<T>(serializer, 0, 0);
    test_value<T>(serializer, 1, 1);

    serializer.reset(new Serializer<T>(10, 10, 10));
    test_value<T>(serializer, 10, 10);
    serializer.reset(new Serializer<T>(10, 3, 10));
    test_value<T>(serializer, 10, 3);
    test_value<T>(serializer, 3, 3);
    test_value<T>(serializer, 5, 3);
    serializer.reset(new Serializer<T>(10, 3, 1));
    test_value<T>(serializer, 10, 3);
    test_value<T>(serializer, 3, 3);
    test_value<T>(serializer, 5, 3);

    // Test a normal serializer that has min = 0 with more
    // than enough bitspace.
    serializer.reset(new Serializer<T>(0, 10, 10));
    for (unsigned int i = 0; i <= 10; i++) {
        test_value<T>(serializer, i, i);
    }
    // Test beyond bounds
    test_value<T>(serializer, 11, 10);

    // Test a normal serializer that starts at a nonzero value,
    // with more than enough bitspace
    serializer.reset(new Serializer<T>(3, 10, 10));
    for (unsigned int i = 3; i <= 10; i++) {
        test_value<T>(serializer, i, i);
    }
    // Test beyond bounds
    test_value<T>(serializer, 2, 3);

    // Test a normal serializer that starts at a zero value,
    // but with restricted bitspace
    serializer.reset(new Serializer<T>(0, 10, 3));
    test_value<T>(serializer, 0, 0);
    test_value<T>(serializer, 1, 0);
    test_value<T>(serializer, 2, 2);
    test_value<T>(serializer, 3, 2);
    test_value<T>(serializer, 4, 4);
    test_value<T>(serializer, 5, 4);

    // Test string-based deserialization
    serializer.reset(new Serializer<T>(0, 10, 3));
    auto val_ptr = std::make_shared<T>();

    TEST_ASSERT(serializer->deserialize("0", val_ptr));
    serializer->deserialize(val_ptr);
    TEST_ASSERT_EQUAL(0, *val_ptr);
    TEST_ASSERT(serializer->deserialize("1", val_ptr));
    serializer->deserialize(val_ptr);
    TEST_ASSERT_EQUAL(0, *val_ptr);
}

/**
 * @brief Verify that the unsigned int serializer properly encapsulates integers of various sizes.
 *
 * Success criteria:
 * - Bitsets with more bits that needed to fully contain the range do not lose resolution.
 * - Bitsets with fewer bits than needed to fully contain the range of the serializer lose
 *   resolution in a predictable way.
 * - Ranges that start with zero work just as well as ranges that don't start at zero.
 */
void test_uint_serializer() { test_int_serializer<unsigned int>(); }

/**
 * @brief Verify that the signed int serializer properly encapsulates integers of various sizes.
 *
 * Success criteria:
 * - Pass same tests as unsigned int.
 * - Ranges that start at negative values work just as well as other range starts.
 */
void test_sint_serializer() {
    // Verify that unsigned int serializer tests work just fine with
    // signed int serializers
    test_int_serializer<signed int>();

    // Test a serializer beyond its bounds, in the negative direction
    std::shared_ptr<Serializer<signed int>> serializer;
    serializer.reset(new Serializer<signed int>(0, 10, 10));
    test_value<signed int>(serializer, -1, 0);

    // Test serializer signed-value edge-case initializations
    serializer.reset(new Serializer<signed int>(-1, 0, 0));
    test_value<signed int>(serializer, 0, -1);
    test_value<signed int>(serializer, -1, -1);
    serializer.reset(new Serializer<signed int>(-1, 0, 1));
    test_value<signed int>(serializer, -1, -1);
    test_value<signed int>(serializer, 0, 0);

    serializer.reset(new Serializer<signed int>(-10, -10, 10));
    test_value<signed int>(serializer, -10, -10);
    serializer.reset(new Serializer<signed int>(10, -3, 10));
    test_value<signed int>(serializer, 12, -3);
    test_value<signed int>(serializer, 10, -3);
    test_value<signed int>(serializer, 3, -3);
    test_value<signed int>(serializer, 5, -3);
    test_value<signed int>(serializer, -1, -3);
    test_value<signed int>(serializer, -5, -3);
    serializer.reset(new Serializer<signed int>(10, -3, 1));
    test_value<signed int>(serializer, 12, -3);
    test_value<signed int>(serializer, 10, -3);
    test_value<signed int>(serializer, 3, -3);
    test_value<signed int>(serializer, 5, -3);
    test_value<signed int>(serializer, -1, -3);
    test_value<signed int>(serializer, -5, -3);

    // Test serializer signed-value normal initializations
    serializer.reset(new Serializer<signed int>(-1, 10, 10));
    test_value<signed int>(serializer, -1, -1);
    serializer.reset(new Serializer<signed int>(-1, 10, 1));
    test_value<signed int>(serializer, -1, -1);
    test_value<signed int>(serializer, 3, -1);
    test_value<signed int>(serializer, 8, -1);
    test_value<signed int>(serializer, 10, 10);
    serializer.reset(new Serializer<signed int>(-3, -1, 10));
    test_value<signed int>(serializer, -2, -2);
    test_value<signed int>(serializer, -3, -3);
    serializer.reset(new Serializer<signed int>(-5, -1, 1));
    test_value<signed int>(serializer, -5, -5);
    test_value<signed int>(serializer, -4, -5);
    test_value<signed int>(serializer, -2, -5);
    test_value<signed int>(serializer, -1, -1);

    // Test string-based deserialization
    serializer.reset(new Serializer<signed int>(-1, 10, 1));
    auto val_ptr = std::make_shared<signed int>();

    TEST_ASSERT(serializer->deserialize("-1", val_ptr));
    serializer->deserialize(val_ptr);
    TEST_ASSERT_EQUAL(-1, *val_ptr);
    TEST_ASSERT(serializer->deserialize("3", val_ptr));
    serializer->deserialize(val_ptr);
    TEST_ASSERT_EQUAL(-1, *val_ptr);
    TEST_ASSERT(serializer->deserialize("10", val_ptr));
    serializer->deserialize(val_ptr);
    TEST_ASSERT_EQUAL(10, *val_ptr);
}

template <typename T>
void test_float_or_double_serializer() {
    std::shared_ptr<Serializer<T>> serializer;
    auto val_ptr = std::make_shared<T>();
    T threshold;

    // Test edge-case initializations
    // TODO
    serializer.reset(new Serializer<T>(0, 0, 5));
    threshold = 0;
    test_value_float_or_double<T>(serializer, 0, 0, threshold);
    test_value_float_or_double<T>(serializer, -1, 0, threshold);
    test_value_float_or_double<T>(serializer, 2, 0, threshold);

    // Test normal initializations
    serializer.reset(new Serializer<T>(0, 3, 5));
    threshold = 3.0 / 31;
    for (size_t i = 0; i < 100; i++) {
        T x = i * 3.0 / 100;

        test_value_float_or_double<T>(serializer, x, x, threshold);
    }
    test_value_float_or_double<T>(serializer, 4, 3, threshold);
    test_value_float_or_double<T>(serializer, -1, 0, threshold);

    serializer.reset(new Serializer<T>(-1, 3, 6));
    threshold = 4.0 / 63;
    for (size_t i = 0; i < 1000; i++) {
        T x = -1.0 + i * 4.0 / 1000;
        test_value_float_or_double<T>(serializer, x, x, threshold);
    }
    test_value_float_or_double<T>(serializer, 4, 3, threshold);
    test_value_float_or_double<T>(serializer, -2, -1, threshold);

    // Test string-based deserialization
    for (size_t i = 0; i < 1000; i++) {
        T x = -1.0 + i * 4.0 / 1000;

        std::ostringstream ss;
        ss << x;
        const std::string str = ss.str();
        TEST_ASSERT(serializer->deserialize(str.c_str(), val_ptr));
        serializer->deserialize(val_ptr);
        if (std::is_same<T, float>::value) {
            TEST_ASSERT_FLOAT_WITHIN(threshold, x, *val_ptr);
        } else {
            TEST_ASSERT_DOUBLE_WITHIN(threshold, x, *val_ptr);
        }
    }
}

/**
 * @brief Verify that the float serializer properly encapsulates float values of various sizes.
 *
 * Success criteria: same as signed int serializer. Comparisons for accuracy, however, cannot be an
 * equality--they must be based on resolution thresholds.
 */
void test_float_serializer() { test_float_or_double_serializer<float>(); }

/**
 * @brief Verify that the double serializer properly encapsulates double values of various sizes.
 *
 * Success criteria: same as float.
 */
void test_double_serializer() { test_float_or_double_serializer<double>(); }

/**
 * @brief Verify that the float vector serializer properly encapsulates float vectors of various
 * sizes.
 */
void test_f_vec_serializer() { TEST_IGNORE_MESSAGE("Needs to be implemented."); }

/**
 * @brief Verify that the double vector serializer properly encapsulates double vectors of various
 * sizes.
 */
void test_d_vec_serializer() { TEST_IGNORE_MESSAGE("Needs to be implemented."); }

/**
 * @brief Verify that the float quaternion serializer properly encapsulates float quaternions of
 * various sizes.
 */
void test_f_quat_serializer() { TEST_IGNORE_MESSAGE("Needs to be implemented."); }

/**
 * @brief Verify that the double quaternion serializer properly encapsulates double quaternions
 * of various sizes.
 */
void test_d_quat_serializer() { TEST_IGNORE_MESSAGE("Needs to be implemented."); }

/**
 * @brief Verify that the GPS time serializer properly encapsulates various values of GPS time.
 */
void test_gpstime_serializer() { TEST_IGNORE_MESSAGE("Needs to be implemented."); }

void test_serializers() {
    UNITY_BEGIN();
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
    UNITY_END();
}

#ifdef DESKTOP
int main(int argc, char* argv[]) {
    test_serializers();
    return 0;
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_serializers();
}

void loop() {}
#endif
