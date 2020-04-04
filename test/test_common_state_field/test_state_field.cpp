#include <common/StateField.hpp>

#include <unity.h>

// Test setter and getter for state field.
void test_internal_state_field() {
    InternalStateField<unsigned int> isf("field");
    TEST_ASSERT_FALSE(isf.is_readable());
    TEST_ASSERT_FALSE(isf.is_writable());
    TEST_ASSERT_EQUAL_STRING("field", isf.name().c_str());

    isf.set(2);
    TEST_ASSERT_EQUAL(isf.get(), 2);
}

// Helper function for testing member functions of a serializable
// state field.
void test_serializable_state_field(SerializableStateField<unsigned int>* field) {
    TEST_ASSERT_EQUAL(0, field->get_serializer_min());
    TEST_ASSERT_EQUAL(10, field->get_serializer_max());
    TEST_ASSERT_EQUAL(10, field->bitsize());

    field->set(2);
    TEST_ASSERT_EQUAL(2, field->get());

    // We can get the bit array and read the serialized value
    // of the state field.
    field->serialize();
    const bit_array& ba = field->get_bit_array();
    TEST_ASSERT_EQUAL(0, ba[0]);
    TEST_ASSERT_EQUAL(1, ba[1]);

    // Setting the bit array doesn't work if the array isn't the same
    // length as the state field's serializer's bit array length.
    bit_array ba2(std::vector<bool>({1, 1, 0}));
    field->set_bit_array(ba2);
    field->deserialize();
    TEST_ASSERT_NOT_EQUAL(6, field->get());
    TEST_ASSERT_EQUAL(2, field->get());

    // Setting the bit array works if we feed in a bit array of the right
    // size. The bitsize is 10 (see the serializers in test_readable_state_field
    // and test_writable_state_field) so this is what we'll use.
    bit_array ba3(std::vector<bool>({0,1,1,0,0,0,0,0,0,0}));
    field->set_bit_array(ba3);
    field->deserialize();
    TEST_ASSERT_EQUAL(6, field->get());

    // Deserializing a string containing a value for the field should work.
    field->deserialize("3");
    TEST_ASSERT_EQUAL(3, field->get());
    TEST_ASSERT_EQUAL_STRING("3", field->print());
}

// Test setters, getters, and serialization properties of a readable
// state field.
void test_readable_state_field() {
    Serializer<unsigned int> serializer(0, 10, 10);
    ReadableStateField<unsigned int> rsf("field", serializer);
    TEST_ASSERT(rsf.is_readable());
    TEST_ASSERT_FALSE(rsf.is_writable());
    TEST_ASSERT_EQUAL_STRING("field", rsf.name().c_str());
    TEST_ASSERT(rsf.is_readable_field());

    test_serializable_state_field(&rsf);
}

// Test setters, getters, and serialization properties of a writable
// state field.
void test_writable_state_field() {
    Serializer<unsigned int> serializer(0, 10, 10);
    WritableStateField<unsigned int> wsf("field", serializer);
    TEST_ASSERT(wsf.is_readable());
    TEST_ASSERT(wsf.is_writable());
    TEST_ASSERT_EQUAL_STRING("field", wsf.name().c_str());
    TEST_ASSERT(wsf.is_writable_field());

    test_serializable_state_field(&wsf);
}

void test_eeprom_save_period() {
    WritableStateField<bool> sf1("field", Serializer<bool>());
    TEST_ASSERT_EQUAL(0, sf1.eeprom_save_period());

    WritableStateField<bool> sf2("field", Serializer<bool>(), 2);
    TEST_ASSERT_EQUAL(2, sf2.eeprom_save_period());
}

void test_get_set_eeprom() {
    WritableStateField<bool> bool_sf("field", Serializer<bool>(), 2);
    bool_sf.set(false);
    TEST_ASSERT_EQUAL(0, bool_sf.get_eeprom_repr());
    bool_sf.set(true);
    TEST_ASSERT_EQUAL(1, bool_sf.get_eeprom_repr());
    bool_sf.set_from_eeprom(0);
    TEST_ASSERT_FALSE(bool_sf.get());
    bool_sf.set_from_eeprom(1);
    TEST_ASSERT_TRUE(bool_sf.get());

    WritableStateField<signed char> schar_sf("field", Serializer<signed char>(), 2);
    schar_sf.set(0);
    TEST_ASSERT_EQUAL(0, schar_sf.get_eeprom_repr());
    schar_sf.set(127);
    TEST_ASSERT_EQUAL(127, schar_sf.get_eeprom_repr());
    schar_sf.set(-1);
    TEST_ASSERT_EQUAL(255, schar_sf.get_eeprom_repr());
    schar_sf.set(-128);
    TEST_ASSERT_EQUAL(128, schar_sf.get_eeprom_repr());
    schar_sf.set_from_eeprom(0);
    TEST_ASSERT_EQUAL(0, schar_sf.get());
    schar_sf.set_from_eeprom(127);
    TEST_ASSERT_EQUAL(127, schar_sf.get());
    schar_sf.set_from_eeprom(255);
    TEST_ASSERT_EQUAL(-1, schar_sf.get());
    schar_sf.set_from_eeprom(128);
    TEST_ASSERT_EQUAL(-128, schar_sf.get());

    WritableStateField<unsigned char> uchar_sf("field", Serializer<unsigned char>(), 2);
    uchar_sf.set(0);
    TEST_ASSERT_EQUAL(0, uchar_sf.get_eeprom_repr());
    uchar_sf.set(127);
    TEST_ASSERT_EQUAL(127, uchar_sf.get_eeprom_repr());
    uchar_sf.set(255);
    TEST_ASSERT_EQUAL(255, uchar_sf.get_eeprom_repr());
    uchar_sf.set_from_eeprom(0);
    TEST_ASSERT_EQUAL(0, uchar_sf.get());
    uchar_sf.set_from_eeprom(127);
    TEST_ASSERT_EQUAL(127, uchar_sf.get());
    uchar_sf.set_from_eeprom(255);
    TEST_ASSERT_EQUAL(255, uchar_sf.get());

    WritableStateField<signed int> sint_sf("field", Serializer<signed int>(), 2);
    sint_sf.set(0);
    TEST_ASSERT_EQUAL(0, sint_sf.get_eeprom_repr());
    sint_sf.set(2147483647);
    TEST_ASSERT_EQUAL(2147483647, sint_sf.get_eeprom_repr());
    sint_sf.set(-1);
    TEST_ASSERT_EQUAL(4294967295, sint_sf.get_eeprom_repr());
    sint_sf.set(-2147483648);
    TEST_ASSERT_EQUAL(2147483648, sint_sf.get_eeprom_repr());
    sint_sf.set_from_eeprom(0);
    TEST_ASSERT_EQUAL(0, sint_sf.get());
    sint_sf.set_from_eeprom(2147483647);
    TEST_ASSERT_EQUAL(2147483647, sint_sf.get());
    sint_sf.set_from_eeprom(4294967295);
    TEST_ASSERT_EQUAL(-1, sint_sf.get());
    sint_sf.set_from_eeprom(2147483648);
    TEST_ASSERT_EQUAL(-2147483648, sint_sf.get());

    WritableStateField<unsigned int> uint_sf("field", Serializer<unsigned int>(), 2);
    uint_sf.set(0);
    TEST_ASSERT_EQUAL(0, uint_sf.get_eeprom_repr());
    uint_sf.set(2147483647);
    TEST_ASSERT_EQUAL(2147483647, uint_sf.get_eeprom_repr());
    uint_sf.set(4294967295);
    TEST_ASSERT_EQUAL(4294967295, uint_sf.get_eeprom_repr());
    uint_sf.set_from_eeprom(0);
    TEST_ASSERT_EQUAL(0, uint_sf.get());
    uint_sf.set_from_eeprom(2147483647);
    TEST_ASSERT_EQUAL(2147483647, uint_sf.get());
    uint_sf.set_from_eeprom(4294967295);
    TEST_ASSERT_EQUAL(4294967295, uint_sf.get());
}

void test_state_field() {
    UNITY_BEGIN();
    RUN_TEST(test_internal_state_field);
    RUN_TEST(test_readable_state_field);
    RUN_TEST(test_writable_state_field);
    RUN_TEST(test_eeprom_save_period);
    RUN_TEST(test_get_set_eeprom);
    UNITY_END();
}

#ifdef DESKTOP
int main(int argc, char *argv[]) {
    test_state_field();
    return 0;
}
#else
#include <Arduino.h>
void setup() {
    delay(10000);
    Serial.begin(9600);
    test_state_field();
}

void loop() {}
#endif
