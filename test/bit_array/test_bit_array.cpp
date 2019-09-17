#include <unity.h>
#include <fixed_array.hpp>

void test_bitarray_constructors() {
    // Default constructor
    std::shared_ptr<bit_array> arr;
    arr.reset(new bit_array);
    TEST_ASSERT_EQUAL(0, arr->size());

    // Size constructor
    arr.reset(new bit_array(10));
    TEST_ASSERT_EQUAL(10, arr->size());

    // Copy constructor (also tests assignment operator)
    arr->at(0) = 1;
    bit_array arr2(*arr);
    TEST_ASSERT_EQUAL(10, arr2.size());
    TEST_ASSERT_EQUAL(1, arr2[0]);
}

void test_bitarray_set_int() {
    bit_array arr(3);

    // Set a "normal" integer value.
    TEST_ASSERT(arr.set_int(6));
    TEST_ASSERT_EQUAL(0, arr.at(0));
    TEST_ASSERT_EQUAL(1, arr.at(1));
    TEST_ASSERT_EQUAL(1, arr.at(2));

    // Set to zero
    TEST_ASSERT(arr.set_int(0));
    TEST_ASSERT_EQUAL(0, arr.at(0));
    TEST_ASSERT_EQUAL(0, arr.at(1));
    TEST_ASSERT_EQUAL(0, arr.at(2));

    // Try to set a value beyond the bounds of what can be stored.
    // The same value as the previous one should be kept.
    TEST_ASSERT_FALSE(arr.set_int(8));
    TEST_ASSERT_EQUAL(0, arr.at(0));
    TEST_ASSERT_EQUAL(0, arr.at(1));
    TEST_ASSERT_EQUAL(0, arr.at(2));
}

void test_bitarray_convert_to_integer() {
    bit_array arr(3);

    arr.set_int(0);
    TEST_ASSERT_EQUAL(0, arr.to_uint());

    arr.set_int(6);
    TEST_ASSERT_EQUAL(6, arr.to_uint());

    arr.set_int(8);
    TEST_ASSERT_EQUAL(6, arr.to_uint());
}

void test_bitarray_write_to_string() {
    bit_array arr(12);
    arr.set_int(4095);  // All the bits will be ones

    unsigned char small_buf[1] = {0};
    arr.to_string(small_buf, sizeof(small_buf));
    TEST_ASSERT_EQUAL(0, small_buf[0]);  // Nothing will have changed; buffer is too small.

    unsigned char buf[2] = {0};
    arr.to_string(buf, sizeof(buf));
    // String will be:
    // 00001111 11111111
    // (byte 1) (byte 0)
    TEST_ASSERT_EQUAL(255, buf[0]);
    TEST_ASSERT_EQUAL(15, buf[1]);
}

void test_bit_array() {
    UNITY_BEGIN();
    RUN_TEST(test_bitarray_constructors);
    RUN_TEST(test_bitarray_set_int);
    RUN_TEST(test_bitarray_convert_to_integer);
    RUN_TEST(test_bitarray_write_to_string);
    UNITY_END();
}

#ifdef DESKTOP
int main(int argc, char *argv[]) {
    test_bit_array();
    return 0;
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_bit_array();
}

void loop() {}
#endif
