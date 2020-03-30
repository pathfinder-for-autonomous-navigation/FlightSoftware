#include <unity.h>
#include <common/fixed_array.hpp>
#include <memory>

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

    // Copy constructor from std::vector (also tests assignment operator)
    std::vector<bool> arr3(8);
    arr3[5] = 1;
    bit_array arr4(arr3);
    TEST_ASSERT_EQUAL(8, arr4.size());
    TEST_ASSERT_EQUAL(1, arr4[5]);
    TEST_ASSERT_NOT_EQUAL(&arr4, &arr3);
}

void test_bitarray_set_int() {
    bit_array arr(3);

    // Set a "normal" integer value.
    TEST_ASSERT(arr.set_int(6));
    TEST_ASSERT_EQUAL(0, arr[0]);
    TEST_ASSERT_EQUAL(1, arr[1]);
    TEST_ASSERT_EQUAL(1, arr[2]);

    // Set to zero
    TEST_ASSERT(arr.set_int(0));
    TEST_ASSERT_EQUAL(0, arr[0]);
    TEST_ASSERT_EQUAL(0, arr[1]);
    TEST_ASSERT_EQUAL(0, arr[2]);

    // Set at the max.
    TEST_ASSERT(arr.set_int(7));
    TEST_ASSERT_EQUAL(1, arr[0]);
    TEST_ASSERT_EQUAL(1, arr[1]);
    TEST_ASSERT_EQUAL(1, arr[2]);
    
    arr.set_int(0);
    // Try to set a value beyond the bounds of what can be stored.
    // The same value as the previous one should be kept.
    TEST_ASSERT_FALSE(arr.set_int(8));
    TEST_ASSERT_EQUAL(0, arr[0]);
    TEST_ASSERT_EQUAL(0, arr[1]);
    TEST_ASSERT_EQUAL(0, arr[2]);
}

void test_bitarray_convert_to_integer() {
    bit_array arr(3);

    arr.set_int(0);
    TEST_ASSERT_EQUAL(0, arr.to_uint());

    arr.set_int(6);
    TEST_ASSERT_EQUAL(6, arr.to_uint());

    arr.set_int(8);
    // Setting the value equal to 8 fails, since
    // there is not enough space in the bitset.
    TEST_ASSERT_EQUAL(6, arr.to_uint());
}

void test_bitarray_write_to_string() {
    bit_array arr(12);
    arr.set_int(4095);  // arr = 111111111111

    /** Test writing to a buffer with zero offset **/
    char* buf = new char[2];
    buf[0] = 0; buf[1] = 0;
    arr.to_string(buf, 0);
    // String will be:
    // 11111111 11110000
    // (byte 0) (byte 1)
    TEST_ASSERT_EQUAL_UINT8(255, buf[0]);
    TEST_ASSERT_EQUAL_UINT8(240, buf[1]);

    /** Test writing to a buffer with some offset
        and with some room left over **/
    buf[0] = 0; buf[1] = 0;
    arr.to_string(buf, 2);
    // String will be:
    // 00111111 11111100
    TEST_ASSERT_EQUAL_UINT8(63, buf[0]);
    TEST_ASSERT_EQUAL_UINT8(252, buf[1]);

    /** Test writing to a buffer that already
        has some data in it **/
    bit_array arr2(4);
    arr2.set_int(15); // arr = 1111
    buf[0] = -128; buf[1] = 7;
    arr2.to_string(buf, 3);
    // Original string was:
    // 10000000 0000111
    // New string will be:
    // 10011110 0000111
    TEST_ASSERT_EQUAL_UINT8(158, buf[0]);
    TEST_ASSERT_EQUAL_UINT8(7, buf[1]);

    /** Test writing a subset of the bit array **/
    buf[0] = -128; buf[1] = 7;
    arr2.to_string(buf, 3, 1, 3);
    // Original string was:
    // 10000000 0000111
    // New string will be:
    // 10011000 0000111
    TEST_ASSERT_EQUAL_UINT8(152, buf[0]);
    TEST_ASSERT_EQUAL_UINT8(7, buf[1]);

    delete[] buf;
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
    delay(10000);
    Serial.begin(9600);
    test_bit_array();
}

void loop() {}
#endif
