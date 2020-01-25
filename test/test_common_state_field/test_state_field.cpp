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

void test_state_field() {
    UNITY_BEGIN();
    RUN_TEST(test_internal_state_field);
    RUN_TEST(test_readable_state_field);
    RUN_TEST(test_writable_state_field);
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
    delay(2000);
    Serial.begin(9600);
    test_state_field();
}

void loop() {}
#endif
