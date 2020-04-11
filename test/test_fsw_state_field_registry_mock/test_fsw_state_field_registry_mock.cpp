#include "../StateFieldRegistryMock.hpp"

#include <unity.h>

void test_valid_initialization() {
    StateFieldRegistryMock registry; 
}

/**
 * @brief Test creation of an internal state field.
 */
void test_create_internal_field() {
    StateFieldRegistryMock registry;

    registry.create_internal_field<bool>("foo");
    TEST_ASSERT_NOT_NULL(registry.find_internal_field("foo"));
    TEST_ASSERT_NULL(registry.find_readable_field("foo"));
    TEST_ASSERT_NULL(registry.find_writable_field("foo"));
}

/**
 * @brief Test creation functions that don't require any arguments
 * besides the name of the field.
 */
void test_create_readable_field_noargs() {
    StateFieldRegistryMock registry;

    registry.create_readable_field<bool>("foo");
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo"));
    TEST_ASSERT_NULL(registry.find_writable_field("foo"));

    registry.create_readable_field<gps_time_t>("foo2");
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo2"));
    TEST_ASSERT_NULL(registry.find_writable_field("foo2"));

    registry.create_readable_field<f_quat_t>("foo3");
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo3"));
    TEST_ASSERT_NULL(registry.find_writable_field("foo3"));

    registry.create_readable_field<d_quat_t>("foo4");
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo4"));
    TEST_ASSERT_NULL(registry.find_writable_field("foo4"));
}

void test_create_writable_field_noargs() {
    StateFieldRegistryMock registry;

    registry.create_writable_field<bool>("foo");
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("foo"));

    registry.create_writable_field<gps_time_t>("foo2");
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo2"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("foo2"));

    registry.create_writable_field<f_quat_t>("foo3");
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo3"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("foo3"));

    registry.create_readable_field<d_quat_t>("foo4");
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo4"));
    TEST_ASSERT_NULL(registry.find_writable_field("foo4"));
}

/**
 * @brief Test create functions that require numerical arguments
 * about serialization parameters.
 */
void test_create_readable_field_args() {
    StateFieldRegistryMock registry;

    registry.create_readable_field<signed int>("foo", -1, 10, 4);
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo"));
    TEST_ASSERT_NULL(registry.find_writable_field("foo"));

    registry.create_readable_field<unsigned int>("foo2", 1, 10, 4);
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo2"));
    TEST_ASSERT_NULL(registry.find_writable_field("foo2"));

    registry.create_readable_field<signed char>("foo5", -1, 10, 4);
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo5"));
    TEST_ASSERT_NULL(registry.find_writable_field("foo5"));

    registry.create_readable_field<unsigned char>("foo6", 1, 10, 4);
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo6"));
    TEST_ASSERT_NULL(registry.find_writable_field("foo6"));

    registry.create_readable_field<float>("foo3", 2.2, 10, 4);
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo3"));
    TEST_ASSERT_NULL(registry.find_writable_field("foo3"));

    registry.create_readable_field<double>("foo4", 2.2, 10, 4);
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo4"));
    TEST_ASSERT_NULL(registry.find_writable_field("foo4"));
}

void test_create_writable_field_args() {
    StateFieldRegistryMock registry;

    registry.create_writable_field<signed int>("foo", -1, 10, 4);
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("foo"));

    registry.create_writable_field<unsigned int>("foo2", 1, 10, 4);
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo2"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("foo2"));

    registry.create_writable_field<signed char>("foo5", -1, 10, 4);
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo5"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("foo5"));

    registry.create_writable_field<unsigned char>("foo6", 1, 10, 4);
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo6"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("foo6"));

    registry.create_writable_field<float>("foo3", 2.2, 10, 4);
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo3"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("foo3"));

    registry.create_writable_field<double>("foo4", 2.2, 10, 4);
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo4"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("foo4"));
}

/**
 * @brief Test vector state field creation functions.
 */
void test_create_readable_vector_field_args() {
    StateFieldRegistryMock registry;

    registry.create_readable_vector_field<float>("foo", 0, 10, 40);
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo"));
    TEST_ASSERT_NULL(registry.find_writable_field("foo"));

    registry.create_readable_vector_field<double>("foo2", 0, 10, 40);
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo2"));
    TEST_ASSERT_NULL(registry.find_writable_field("foo2"));
}

void test_create_writable_vector_field_args() {
    StateFieldRegistryMock registry;

    registry.create_writable_vector_field<float>("foo", 0, 10, 40);
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("foo"));

    registry.create_writable_vector_field<double>("foo2", 0, 10, 40);
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo2"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("foo2"));
}

void test_create_readable_eeprom_saved_field() {
    StateFieldRegistryMock registry;
    auto ptr = registry.create_readable_field<unsigned int>("foo");
    TEST_ASSERT_EQUAL(0, ptr->eeprom_save_period());

    // Test creating fields of all possible numbers of parameters
    ptr = registry.create_readable_field<unsigned int, 4>("foo");
    TEST_ASSERT_EQUAL(4, ptr->eeprom_save_period());
    ptr = registry.create_readable_field<unsigned int, 4>("foo", 2);
    TEST_ASSERT_GREATER_THAN(0, ptr->eeprom_save_period());
    ptr = registry.create_readable_field<unsigned int, 4>("foo", 2, 3);
    TEST_ASSERT_GREATER_THAN(0, ptr->eeprom_save_period());
    ptr = registry.create_readable_field<unsigned int, 4>("foo", 2, 3, 5);
    TEST_ASSERT_GREATER_THAN(0, ptr->eeprom_save_period());

    // Test creating fields of all EEPROM-saveable kinds
    auto ptr2 = registry.create_readable_field<signed int, 4>("foo", -2, 2, 5);
    auto ptr3 = registry.create_readable_field<unsigned char, 4>("foo");
    auto ptr4 = registry.create_readable_field<signed char, 4>("foo", -2, 2, 5);
    auto ptr5 = registry.create_readable_field<bool, 4>("foo");
    TEST_ASSERT_GREATER_THAN(0, ptr2->eeprom_save_period());
    TEST_ASSERT_GREATER_THAN(0, ptr3->eeprom_save_period());
    TEST_ASSERT_GREATER_THAN(0, ptr4->eeprom_save_period());
    TEST_ASSERT_GREATER_THAN(0, ptr5->eeprom_save_period());
}

void test_create_writable_eeprom_saved_field() {
    StateFieldRegistryMock registry;
    auto ptr = registry.create_writable_field<unsigned int>("foo");
    TEST_ASSERT_EQUAL(0, ptr->eeprom_save_period());

    // Test creating fields of all possible numbers of parameters
    ptr = registry.create_writable_field<unsigned int, 4>("foo");
    TEST_ASSERT_EQUAL(4, ptr->eeprom_save_period());
    ptr = registry.create_writable_field<unsigned int, 4>("foo", 2);
    TEST_ASSERT_GREATER_THAN(0, ptr->eeprom_save_period());
    ptr = registry.create_writable_field<unsigned int, 4>("foo", 2, 3);
    TEST_ASSERT_GREATER_THAN(0, ptr->eeprom_save_period());
    ptr = registry.create_writable_field<unsigned int, 4>("foo", 2, 3, 5);
    TEST_ASSERT_GREATER_THAN(0, ptr->eeprom_save_period());

    // Test creating fields of all EEPROM-saveable kinds
    auto ptr2 = registry.create_writable_field<signed int, 4>("foo", -2, 2, 5);
    auto ptr3 = registry.create_writable_field<unsigned char, 4>("foo");
    auto ptr4 = registry.create_writable_field<signed char, 4>("foo", -2, 2, 5);
    auto ptr5 = registry.create_writable_field<bool, 4>("foo");
    TEST_ASSERT_GREATER_THAN(0, ptr2->eeprom_save_period());
    TEST_ASSERT_GREATER_THAN(0, ptr3->eeprom_save_period());
    TEST_ASSERT_GREATER_THAN(0, ptr4->eeprom_save_period());
    TEST_ASSERT_GREATER_THAN(0, ptr5->eeprom_save_period());
}

/**
 * @brief This print function will be used to instatiate an event, which will be added to the
 * registry in test_create_event()
 */
static const char* print_fn(const unsigned int ccno, std::vector<ReadableStateFieldBase*>& data) {
    static char print_data[40];
    memset(print_data, 0, 40);
    ReadableStateField<bool>* datafield1_f = static_cast<ReadableStateField<bool>*>(data[0]);
    ReadableStateField<bool>* datafield2_f = static_cast<ReadableStateField<bool>*>(data[1]);
    sprintf((char*)print_data,  "E: time: %d, data: %d, %d", ccno, datafield1_f->get(), datafield2_f->get());
    return print_data;
}

void test_create_event() {
    StateFieldRegistryMock registry;
    registry.create_readable_field<signed int>("field1", -1, 10, 4);
    registry.create_readable_field<signed int>("field2", -1, 10, 4);
    ReadableStateFieldBase* data1_fp=registry.find_readable_field("field1");
    ReadableStateFieldBase* data2_fp=registry.find_readable_field("field2");
    std::vector<ReadableStateFieldBase*> event_data={data1_fp, data2_fp};

    registry.create_event("event", event_data, print_fn);
    TEST_ASSERT_NOT_NULL(registry.find_event("event"));
    TEST_ASSERT_NOT_NULL(registry.find_event_t("event"));
}

void test_create_fault() {
    StateFieldRegistryMock registry;

    registry.create_fault("foo", 1, 3);
    TEST_ASSERT_NOT_NULL(registry.find_fault("foo"));
    TEST_ASSERT_NOT_NULL(registry.find_fault_t("foo"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("foo"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("foo.suppress"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("foo.override"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("foo.unsignal"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("foo.persistence"));
}

/**
 * @brief Test clearing the registry.
 */
void test_clear() {
    StateFieldRegistryMock registry;

    registry.create_writable_field<unsigned int>("foo", 0, 10, 4);
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("foo"));

    registry.create_readable_field<unsigned int>("foo2", 0, 10, 4);
    TEST_ASSERT_NOT_NULL(registry.find_readable_field("foo2"));

    registry.create_internal_field<unsigned int>("foo3");
    TEST_ASSERT_NOT_NULL(registry.find_internal_field("foo3"));

    registry.create_fault("foo4", 1, 300);
    TEST_ASSERT_NOT_NULL(registry.find_fault("foo4"));

    registry.create_readable_field<signed int>("field1", -1, 10, 4);
    registry.create_readable_field<signed int>("field2", -1, 10, 4);
    ReadableStateFieldBase* data1_fp=registry.find_readable_field("field1");
    ReadableStateFieldBase* data2_fp=registry.find_readable_field("field2");
    std::vector<ReadableStateFieldBase*> event_data={data1_fp, data2_fp};
    
    registry.create_event("foo5", event_data, print_fn);

    registry.clear();
    TEST_ASSERT_NULL(registry.find_writable_field("foo"));
    TEST_ASSERT_NULL(registry.find_readable_field("foo2"));
    TEST_ASSERT_NULL(registry.find_readable_field("foo3"));
    TEST_ASSERT_NULL(registry.find_writable_field("foo3"));
    TEST_ASSERT_NULL(registry.find_fault("foo4"));
    TEST_ASSERT_NULL(registry.find_event("foo5"));
}

int test_state_field_registry_mock() {
    UNITY_BEGIN();
    RUN_TEST(test_valid_initialization);
    RUN_TEST(test_create_internal_field);
    RUN_TEST(test_create_readable_field_noargs);
    RUN_TEST(test_create_writable_field_noargs);
    RUN_TEST(test_create_readable_field_args);
    RUN_TEST(test_create_writable_field_args);
    RUN_TEST(test_create_readable_eeprom_saved_field);
    RUN_TEST(test_create_writable_eeprom_saved_field);
    RUN_TEST(test_create_readable_vector_field_args);
    RUN_TEST(test_create_writable_vector_field_args);
    RUN_TEST(test_create_event);
    RUN_TEST(test_create_fault);
    RUN_TEST(test_clear);
    return UNITY_END();
}

#ifdef DESKTOP
int main() {
    return test_state_field_registry_mock();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_state_field_registry_mock();
}

void loop() {}
#endif
