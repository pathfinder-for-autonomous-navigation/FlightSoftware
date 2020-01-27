#include <unity.h>
#include <common/Serializer.hpp>
#include <stdlib.h>
#include <memory>

#include <lin.hpp> // for cleaner inner products
// ============================================================================================= //
//                                      Helper methods                                           //
// ============================================================================================= //

// number of vector tests to run
constexpr static int number_of_vec_test = 100;

/**
 * @brief Returns the magnitude of a vector
 * 
 */
template <typename T, size_t N>
T magnitude_of(std::array<T, N>& src){
    lin::Vector<T, N> lin_vec;

    if(N == 4)
        lin_vec = {src[0], src[1], src[2], src[3]};
    else 
        lin_vec = {src[0], src[1], src[2]};
    
    return lin::norm(lin_vec);
}

/**
 * @brief Normalizes a vector or quaternion
 * 
 */
template <typename T, size_t N>
void normalize(std::array<T, N>& src) {

    T magnitude = magnitude_of(src);
    
    for(size_t i = 0; i<N; i++){
        src[i] = src[i] / magnitude;
    }

    return;
}

/**
 * @brief Returns the angle between two quaternions or vectors
 * 
 */
template <typename T, size_t N>
T angle_between(std::array<T, N>& a, std::array<T, N>& b){
    lin::Vector<T, N> lin_a;
    lin::Vector<T, N> lin_b;

    if(N == 4){
        lin_a = {a[0], a[1], a[2], a[3]};
        lin_b = {b[0], b[1], b[2], b[3]};
    }
    else{
        lin_a = {a[0], a[1], a[2]};
        lin_b = {b[0], b[1], b[2]};
    }

    T inner_product = lin::dot(lin_a, lin_b);
    
    T angle;
    if(N == 4){
        // to account for quat could be flipped
        inner_product = std::abs(inner_product);
        angle = std::acos(inner_product)*2.0;
    }
    else
        angle = std::acos(inner_product);

    angle = angle * 360.0 / (2 * 3.14159265);

    return angle;
}

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
void test_value(std::shared_ptr<Serializer<T>>& s,
                std::shared_ptr<Serializer<T>>& d,
                const T val, const T output) {
    T output_val;
    s->serialize(val);
    d->set_bit_array(s->get_bit_array());
    d->deserialize(&output_val);
    TEST_ASSERT_EQUAL(output, output_val);
}

/**
 * @brief Helper method to test serialization and deserialization specifically for floats and
 * doubles.
 */
template <typename T>
void test_value_float_or_double(std::shared_ptr<Serializer<T>>& s, 
                                std::shared_ptr<Serializer<T>>& d, const T val, const T output,
                                const T threshold = 0) {
    static_assert(std::is_same<T, float>::value || 
                  std::is_same<T, double>::value, 
                  "To use this function, the value being tested must either be a float or a double.");

    T output_val;
    s->serialize(val);
    d->set_bit_array(s->get_bit_array());
    d->deserialize(&output_val);

    if (std::is_same<T, float>::value) {
        TEST_ASSERT_FLOAT_WITHIN(threshold, output, output_val);
    } else {
        TEST_ASSERT_DOUBLE_WITHIN(threshold, output, output_val);
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
    // dl is short for downlink
    auto dl_deserializer = std::make_shared<Serializer<bool>>();

    bool dest_ptr;

    // Normal serialize and deserialize
    test_value(serializer, dl_deserializer, true, true);
    test_value(serializer, dl_deserializer, false, false);

    // String-based deserialize
    TEST_ASSERT(serializer->deserialize("true", &dest_ptr));
    TEST_ASSERT(dest_ptr);
    TEST_ASSERT(serializer->deserialize("false", &dest_ptr));
    TEST_ASSERT_FALSE(dest_ptr);
    TEST_ASSERT_FALSE(serializer->deserialize("nonsense", &dest_ptr));

    // Printing
    TEST_ASSERT_EQUAL_STRING(serializer->print(true), "true");
    TEST_ASSERT_EQUAL_STRING(serializer->print(false), "false");

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
    serializer->deserialize(&dest_ptr);
    TEST_ASSERT(dest_ptr);
    val[0] = false;
    serializer->set_bit_array(val);
    serializer->deserialize(&dest_ptr);
    TEST_ASSERT_FALSE(dest_ptr);
}

/**
 * @brief Template function to test common behavior between signed and unsigned int and char serializers.
 *
 * @tparam T
 */
template <typename T>
void test_uint_serializer() {
    static_assert(std::is_same<T, signed int>::value || 
                  std::is_same<T, unsigned int>::value ||
                  std::is_same<T, unsigned char>::value ||
                  std::is_same<T, signed char>::value, 
                  "To use this function, the value being tested must be a integer or character.");

    std::shared_ptr<Serializer<T>> serializer;
    // dl is short for downlink
    std::shared_ptr<Serializer<T>> dl_deserializer;


    /** Test edge-case initializations **/
    serializer.reset(new Serializer<T>(0, 0, 0));
    dl_deserializer.reset(new Serializer<T>(0, 0, 0));

    test_value<T>(serializer, dl_deserializer,  0, 0);
    test_value<T>(serializer, dl_deserializer,  1, 0);

    serializer.reset(new Serializer<T>(0, 1, 0));
    dl_deserializer.reset(new Serializer<T>(0, 1, 0));

    test_value<T>(serializer, dl_deserializer,  0, 0);
    test_value<T>(serializer, dl_deserializer,  1, 0);

    serializer.reset(new Serializer<T>(0, 1, 1));
    dl_deserializer.reset(new Serializer<T>(0, 1, 1));
    test_value<T>(serializer, dl_deserializer,  0, 0);
    test_value<T>(serializer, dl_deserializer,  1, 1);

    serializer.reset(new Serializer<T>(10, 10, 10));
    dl_deserializer.reset(new Serializer<T>(10, 10, 10));

    test_value<T>(serializer, dl_deserializer,  10, 10);
    test_value<T>(serializer, dl_deserializer,  10, 10);
    test_value<T>(serializer, dl_deserializer,  3, 10);
    test_value<T>(serializer, dl_deserializer,  5, 10);

    // Test a normal serializer that has min = 0 with more
    // than enough bitspace.
    serializer.reset(new Serializer<T>(0, 10, 10));
    dl_deserializer.reset(new Serializer<T>(0, 10, 10));

    for (T i = 0; i <= 10; i++) {
        test_value<T>(serializer, dl_deserializer,  i, i);
    }
    // Test beyond bounds
    test_value<T>(serializer, dl_deserializer,  11, 10);

    // Test a normal serializer that starts at a nonzero value,
    // with more than enough bitspace
    serializer.reset(new Serializer<T>(3, 10, 10));
    dl_deserializer.reset(new Serializer<T>(3, 10, 10));

    for (T i = 3; i <= 10; i++) {
        test_value<T>(serializer, dl_deserializer,  i, i);
    }
    // Test beyond bounds
    test_value<T>(serializer, dl_deserializer,  2, 3);

    // Test a normal serializer that starts at a zero value,
    // but with restricted bitspace
    serializer.reset(new Serializer<T>(0, 10, 3));
    dl_deserializer.reset(new Serializer<T>(0, 10, 3));
    test_value<T>(serializer, dl_deserializer,  0, 0);
    test_value<T>(serializer, dl_deserializer,  1, 0);
    test_value<T>(serializer, dl_deserializer,  2, 2);
    test_value<T>(serializer, dl_deserializer,  3, 2);
    test_value<T>(serializer, dl_deserializer,  4, 4);
    test_value<T>(serializer, dl_deserializer,  5, 4);

    // Test string-based deserialization
    serializer.reset(new Serializer<T>(0, 10, 3));

    T val;

    TEST_ASSERT(serializer->deserialize("0", &val));
    TEST_ASSERT_EQUAL_INT(0, val);
    TEST_ASSERT(serializer->deserialize("1", &val));
    TEST_ASSERT_EQUAL_INT(1, val);

    // Test printing
    TEST_ASSERT_EQUAL_STRING("1", serializer->print(1));
}

/**
 * @brief Template function to test common behavior between signed int/char serializers.
 *
 * @tparam T
 */
template<typename T>
void test_sint_serializer() {
    static_assert(std::is_same<T, signed int>::value || 
                  std::is_same<T, signed char>::value, 
                  "To use this function, the value being tested must be a signed integer or character.");

    // Test a serializer beyond its bounds, in the negative direction
    std::shared_ptr<Serializer<T>> serializer;
    // dl is short for downlink
    std::shared_ptr<Serializer<T>> dl_deserializer;

    serializer.reset(new Serializer<T>(0, 10, 10));
    dl_deserializer.reset(new Serializer<T>(0, 10, 10));

    test_value<T>(serializer, dl_deserializer,  -1, 0);

    // Test serializer signed-value edge-case initializations
    serializer.reset(new Serializer<T>(-1, 0, 0));
    dl_deserializer.reset(new Serializer<T>(-1, 0, 0));

    test_value<T>(serializer, dl_deserializer,  0, -1);
    test_value<T>(serializer, dl_deserializer,  -1, -1);

    serializer.reset(new Serializer<T>(-1, 0, 1));
    dl_deserializer.reset(new Serializer<T>(-1, 0, 1));

    test_value<T>(serializer, dl_deserializer,  -1, -1);
    test_value<T>(serializer, dl_deserializer,  0, 0);

    serializer.reset(new Serializer<T>(-10, -10, 10));
    dl_deserializer.reset(new Serializer<T>(-10, -10, 10));

    test_value<T>(serializer, dl_deserializer,  -10, -10);
    test_value<T>(serializer, dl_deserializer,  12, -10);
    test_value<T>(serializer, dl_deserializer,  10, -10);
    test_value<T>(serializer, dl_deserializer,  3, -10);
    test_value<T>(serializer, dl_deserializer,  5, -10);
    test_value<T>(serializer, dl_deserializer,  -1, -10);
    test_value<T>(serializer, dl_deserializer,  -5, -10);

    // Test serializer signed-value normal initializations
    serializer.reset(new Serializer<T>(-1, 10, 10));
    dl_deserializer.reset(new Serializer<T>(-1, 10, 10));
    test_value<T>(serializer, dl_deserializer,  -1, -1);

    serializer.reset(new Serializer<T>(-1, 10, 1));
    dl_deserializer.reset(new Serializer<T>(-1, 10, 1));
    test_value<T>(serializer, dl_deserializer,  -1, -1);
    test_value<T>(serializer, dl_deserializer,  3, -1);
    test_value<T>(serializer, dl_deserializer,  8, -1);
    test_value<T>(serializer, dl_deserializer,  10, 10);
    
    serializer.reset(new Serializer<T>(-3, -1, 10));
    dl_deserializer.reset(new Serializer<T>(-3, -1, 10));
    test_value<T>(serializer, dl_deserializer,  -2, -2);
    test_value<T>(serializer, dl_deserializer,  -3, -3);

    serializer.reset(new Serializer<T>(-5, -1, 1));
    dl_deserializer.reset(new Serializer<T>(-5, -1, 1));
    test_value<T>(serializer, dl_deserializer,  -5, -5);
    test_value<T>(serializer, dl_deserializer,  -4, -5);
    test_value<T>(serializer, dl_deserializer,  -2, -5);
    test_value<T>(serializer, dl_deserializer,  -1, -1);

    // Test string-based deserialization
    serializer.reset(new Serializer<T>(-1, 10, 1));
    T val;

    TEST_ASSERT(serializer->deserialize("-1", &val));
    serializer->deserialize(&val);
    TEST_ASSERT_EQUAL(-1, val);
    TEST_ASSERT(serializer->deserialize("3", &val));
    serializer->deserialize(&val);
    TEST_ASSERT_EQUAL(-1, val);
    TEST_ASSERT(serializer->deserialize("10", &val));
    serializer->deserialize(&val);
    TEST_ASSERT_EQUAL(10, val);
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
void test_unsigned_int_serializer() { test_uint_serializer<unsigned int>(); }

/**
 * @brief Verify that the signed int serializer properly encapsulates integers of various sizes.
 *
 * Success criteria:
 * - Pass same tests as unsigned int.
 * - Ranges that start at negative values work just as well as other range starts.
 */
void test_signed_int_serializer() {
    test_uint_serializer<signed int>();
    test_sint_serializer<signed int>();
}

/**
 * @brief Do the same things as we just did for ints, but for signed and
 * unsigned chars.
 */
void test_unsigned_char_serializer() {
    test_uint_serializer<unsigned char>();
}
void test_signed_char_serializer() {
    test_uint_serializer<signed char>();
    test_sint_serializer<signed char>();
}

template <typename T>
void test_float_or_double_serializer() {
    static_assert(std::is_same<T, float>::value || 
                  std::is_same<T, double>::value, 
                  "To use this function, the value being tested must either be a float or a double.");

    std::shared_ptr<Serializer<T>> serializer;
    // dl is short for downlink
    std::shared_ptr<Serializer<T>> dl_deserializer;
    T threshold;

    // Test edge-case initializations
    // TODO
    serializer.reset(new Serializer<T>(0, 0, 5));
    dl_deserializer.reset(new Serializer<T>(0, 0, 5));
    
    threshold = 0;
    test_value_float_or_double<T>(serializer, dl_deserializer,  0, 0, threshold);
    test_value_float_or_double<T>(serializer, dl_deserializer,  -1, 0, threshold);
    test_value_float_or_double<T>(serializer, dl_deserializer,  2, 0, threshold);

    // Test normal initializations
    serializer.reset(new Serializer<T>(0, 3, 5));
    dl_deserializer.reset(new Serializer<T>(0, 3, 5));

    threshold = 3.0 / 31;
    for (size_t i = 0; i < 100; i++) {
        T x = i * 3.0 / 100;

        test_value_float_or_double<T>(serializer, dl_deserializer,  x, x, threshold);
    }
    test_value_float_or_double<T>(serializer, dl_deserializer,  4, 3, threshold);
    test_value_float_or_double<T>(serializer, dl_deserializer,  -1, 0, threshold);

    serializer.reset(new Serializer<T>(-1, 3, 6));
    dl_deserializer.reset(new Serializer<T>(-1, 3, 6));

    threshold = 4.0 / 63;
    for (size_t i = 0; i < 1000; i++) {
        T x = -1.0 + i * 4.0 / 1000;
        test_value_float_or_double<T>(serializer, dl_deserializer,  x, x, threshold);
    }
    test_value_float_or_double<T>(serializer, dl_deserializer,  4, 3, threshold);
    test_value_float_or_double<T>(serializer, dl_deserializer,  -2, -1, threshold);

    // Test string-based deserialization
    T val;
    for (size_t i = 0; i < 1000; i++) {
        T x = -1.0 + i * 4.0 / 1000;

        char buf[100];
        memset(buf, 0, 100);
        if (std::is_same<T, float>::value) sprintf(buf, "%f", x);
        else sprintf(buf, "%lf", x);
        TEST_ASSERT(serializer->deserialize(buf, &val));
        serializer->deserialize(&val);
        if (std::is_same<T, float>::value) {
            TEST_ASSERT_FLOAT_WITHIN(threshold, x, val);
        } else {
            TEST_ASSERT_DOUBLE_WITHIN(threshold, x, val);
        }
    }

    // Test printing
    TEST_ASSERT_EQUAL_STRING("2.200000", serializer->print(2.2));
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
 *
 */
template<typename T>
void test_vec_serializer() {
    static_assert(std::is_same<T, float>::value || 
                  std::is_same<T, double>::value, 
                  "To use this function, the value being tested must either be a float vector or a"
                  " double vector.");

    using vector_t = typename std::conditional<std::is_same<T, float>::value,
                         f_vector_t,
                         d_vector_t>::type;

    // TODO write serialization initializations for edge cases.
    
    const size_t vec_bitsize = 40;

    // (Deterministically) generate random vectors of magnitude 2, and see if they work 
    // with the serializer.
    // Criterion for functionality: vector is within 0.5 degrees, and within 0.1% of magnitude

    srand(2);
    for(size_t i = 0; i < number_of_vec_test; i++) {
        auto vec_serializer = std::make_shared<Serializer<vector_t>>(0,2,vec_bitsize);
        
        //make the another serialier with same inputs
        auto downlink_deserializer = std::make_shared<Serializer<vector_t>>(0,2,vec_bitsize);
        vector_t result;

        // rand() returns the same thing every time

        // Generate random vector.
        const T x = rand() / T(RAND_MAX) * 2;
        const T t = rand() / T(RAND_MAX) * (2 * 3.14159265);
        const T y = cos(t) * std::sqrt(4 - x*x);
        const T z = sin(t) * std::sqrt(4 - x*x);

        vector_t original = {x, y, z};
        vector_t vec(original);

        vec_serializer->serialize(vec);

        // transfer bit array; analagous to reading in telemetry bit array
        downlink_deserializer->set_bit_array(vec_serializer->get_bit_array());

        downlink_deserializer->deserialize(&result);

        // make a copy of the result
        vector_t original_result(result);

        // normalize the vectors for angle calculation only
        normalize<T, 3>(vec);
        normalize<T, 3>(result);
        T angle = angle_between(vec, result);

        T mag_err = std::abs(magnitude_of(original_result)/magnitude_of(original) - 1.0) ;

        static const char* err_fmt_str_f = "%dth test: Input vector was {%f,%f,%f}; output vector was {%f,%f,%f}; angle: %f; mag_err: %f";
        static const char* err_fmt_str_d = "%dth test: Input vector was {%lf,%lf,%lf}; output vector was {%lf,%lf,%lf}; angle: %lf; mag_err: %lf";
        char err_str[200];
        memset(err_str, 0, 200);
        const char* err_fmt_str = nullptr;
        if (std::is_same<T, float>::value) err_fmt_str = err_fmt_str_f;
        else err_fmt_str = err_fmt_str_d;
        sprintf(err_str, err_fmt_str, i, x, y, z, original_result[0], original_result[1], original_result[2], angle, mag_err);

        // assert less than .1% magnitude error
        TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.001, 0, mag_err, err_str);

        // assert angle has error < 0.5 degrees
        TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.5, 0, angle, err_str);

        // Note no need to check if there are any sign flips,
        // angle_err < 0.5 deg if no sign flips, or if error is small (across an interval)
    }

    // Test deserialization from a string
    auto serializer = std::make_shared<Serializer<vector_t>>(0,2,vec_bitsize);
    vector_t result;
    TEST_ASSERT_FALSE(serializer->deserialize("0.200000,0.500000", &result));
    TEST_ASSERT(serializer->deserialize("3.141592654,0.300000,0.500000", &result));

    if (std::is_same<T, float>::value) {
        TEST_ASSERT_EQUAL_FLOAT(3.141592654, result[0]);
        TEST_ASSERT_EQUAL_FLOAT(0.3, result[1]);
        TEST_ASSERT_EQUAL_FLOAT(0.5, result[2]);
    }
    else {
        TEST_ASSERT_EQUAL_DOUBLE(3.141592654, result[0]);
        TEST_ASSERT_EQUAL_DOUBLE(0.3, result[1]);
        TEST_ASSERT_EQUAL_DOUBLE(0.5, result[2]);
    }

    // Test printing
    vector_t print_val = {2, 4, 2};
    TEST_ASSERT_EQUAL_STRING("2.000000,4.000000,2.000000,", serializer->print(print_val));
}

/**
 * @brief Verify that the float vector serializer properly encapsulates float vectors of various
 * sizes.
 */
void test_f_vec_serializer() {
    test_vec_serializer<float>();
}

/**
 * @brief Verify that the double vector serializer properly encapsulates double vectors of various
 * sizes.
 */
void test_d_vec_serializer() {
    test_vec_serializer<double>();
}

template<typename T>
void test_quat_serializer() {
    static_assert(std::is_same<T, float>::value || 
                  std::is_same<T, double>::value, 
                  "To use this function, the value being tested must either be a float quaternion or a double quaternion.");

    using quat_t = typename std::conditional<std::is_same<T, float>::value,
                         f_quat_t,
                         d_quat_t>::type;

    // TODO write serialization initializations for edge cases. // ?

    // (Deterministically) generate random quaternions, and see if they work 
    // with the serializer.
    // Criterion for functionality: the quaternion that's reported has a displacement from the
    // input quaternion of magnitude at most magnitude_err.
    srand(2);
    for(size_t i = 0; i < number_of_vec_test; i++) {
        auto quat_serializer = std::make_shared<Serializer<quat_t>>();
        auto downlink_deserializer = std::make_shared<Serializer<quat_t>>();

        quat_t result;

        // Generate random quaternion.
        const T t = rand() / T(RAND_MAX) * (2 * 3.14159265);
        const T tt = rand() / T(RAND_MAX) * (2 * 3.14159265);
        const T ux = rand() / T(RAND_MAX) * sin(tt/2);
        const T uy = std::cos(t) * std::sqrt(1 - ux*ux) * std::sin(tt/2);
        const T uz = std::sin(t) * std::sqrt(1 - ux*ux) * std::sin(tt/2);
        const T s = std::cos(tt/2);
        quat_t quat = {ux, uy, uz, s};
        // please note that quat is not normalized at this point

        // serialize will normalize a quaternion argument, but normalize now anyway
        normalize<T, 4>(quat);
        quat_serializer->serialize(quat);

        //downlink_deserializer = quat_serializer;
        downlink_deserializer->set_bit_array(quat_serializer->get_bit_array());

        downlink_deserializer->deserialize(&result);

        // normalize the result (though it should definitely be normalized, or atleast very close)
        normalize<T, 4>(result);
        T angle_err = angle_between<T, 4>(quat, result);

        static const char* err_fmt_str_f = "%dth test: Input quaternion was {%f,%f,%f,%f}; output quaternion was {%f,%f,%f,%f}; angle: %f";
        static const char* err_fmt_str_d = "%dth test: Input quaternion was {%lf,%lf,%lf,%lf}; output quaternion was {%lf,%lf,%lf,%lf}; angle: %lf";
        char err_str[200];
        memset(err_str, 0, 200);
        const char* err_fmt_str = nullptr;
        if (std::is_same<T, float>::value) err_fmt_str = err_fmt_str_f;
        else err_fmt_str = err_fmt_str_d;
        sprintf(err_str, err_fmt_str, i, quat[0], quat[1], quat[2], quat[3], result[0], result[1], result[2], result[3], angle_err);

        TEST_ASSERT_FLOAT_WITHIN_MESSAGE(1.0, 0, angle_err, err_str);

    }

    // Test deserialization from a string
    auto serializer = std::make_shared<Serializer<quat_t>>();
    quat_t result;
    TEST_ASSERT_FALSE(serializer->deserialize("0.200000,0.300000,0.500000", &result));
    TEST_ASSERT(serializer->deserialize("0.200000,0.100000,0.300000,0.500000", &result));
    if (std::is_same<T, float>::value) {
        TEST_ASSERT_EQUAL_FLOAT(0.2, result[0]);
        TEST_ASSERT_EQUAL_FLOAT(0.1, result[1]);
        TEST_ASSERT_EQUAL_FLOAT(0.3, result[2]);
        TEST_ASSERT_EQUAL_FLOAT(0.5, result[3]);
    }
    else {
        TEST_ASSERT_EQUAL_DOUBLE(0.2, result[0]);
        TEST_ASSERT_EQUAL_DOUBLE(0.1, result[1]);
        TEST_ASSERT_EQUAL_DOUBLE(0.3, result[2]);
        TEST_ASSERT_EQUAL_DOUBLE(0.5, result[3]);
    }

    // Test printing
    quat_t print_val = {0.5, 0.1, 0.1, 0.1};
    TEST_ASSERT_EQUAL_STRING("0.500000,0.100000,0.100000,0.100000,", serializer->print(print_val));
}

/**
 * @brief Verify that the float quaternion serializer properly encapsulates float quaternions of
 * various sizes.
 */
void test_f_quat_serializer() {
    test_quat_serializer<float>();
}

/**
 * @brief Verify that the double quaternion serializer properly encapsulates double quaternions
 * of various sizes.
 */
void test_d_quat_serializer() {
    test_quat_serializer<double>();
}

/**
 * @brief Verify that the GPS time serializer properly encapsulates various values of GPS time.
 */
void test_gpstime_serializer() {
    auto gpstime_serializer = std::make_shared<Serializer<gps_time_t>>();
    gps_time_t result;

    // Serialization of uninitialized GPS time
    gps_time_t input;
    gpstime_serializer->serialize(input);
    gpstime_serializer->deserialize(&result);
    TEST_ASSERT_FALSE(result.is_set);

    // Serialization of initialized GPS time
    gps_time_t input2(3,3,3);
    gpstime_serializer->serialize(input2);
    gpstime_serializer->deserialize(&result);
    TEST_ASSERT(result == input2);

    // Deserialization from a string
    gps_time_t input3(2,2,2);
    TEST_ASSERT_FALSE(gpstime_serializer->deserialize("2,2", &result));
    TEST_ASSERT(gpstime_serializer->deserialize("2,2,2", &result));
    TEST_ASSERT(result == input3);

    // Printing
    gps_time_t input4(4,4,4);
    TEST_ASSERT_EQUAL_STRING("4,4,4", gpstime_serializer->print(input4));
}

void test_serializers() {
    UNITY_BEGIN();
    RUN_TEST(test_bool_serializer);
    RUN_TEST(test_unsigned_int_serializer);
    RUN_TEST(test_signed_int_serializer);
    RUN_TEST(test_unsigned_char_serializer);
    RUN_TEST(test_signed_char_serializer);
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
