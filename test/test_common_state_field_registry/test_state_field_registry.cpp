#include <common/StateFieldRegistry.hpp>
#include <common/Event.hpp>
#include <common/Fault.hpp>

#include <unity.h>

void test_foo() {
    StateFieldRegistry registry;
}

/**
 * @brief The print function is used to instantiate an event, which will be added to the
 * registry in test_events()
 */
static const char* print_fn(const unsigned int ccno, std::vector<ReadableStateFieldBase*>& data) {
    static char print_data[40];
    memset(print_data, 0, 40);
    ReadableStateField<bool>* datafield1_f = static_cast<ReadableStateField<bool>*>(data[0]);
    ReadableStateField<bool>* datafield2_f = static_cast<ReadableStateField<bool>*>(data[1]);
    sprintf((char*)print_data,  "E: time: %d, data: %d, %d", ccno, datafield1_f->get(), datafield2_f->get());
    return print_data;
}

void test_events() {
    StateFieldRegistry registry;

    // Define parameters needed to create an event
    ReadableStateField<bool> data1("field1", Serializer<bool>());
    ReadableStateField<bool> data2("field2", Serializer<bool>());
    registry.add_readable_field(&data1);
    registry.add_readable_field(&data2);
    ReadableStateFieldBase* data1_fp=registry.find_readable_field("field1");
    ReadableStateFieldBase* data2_fp=registry.find_readable_field("field2");
    std::vector<ReadableStateFieldBase*> event_data={data1_fp, data2_fp};

    // Add event to registry
    Event e("event", event_data, print_fn);
    TEST_ASSERT_TRUE(registry.add_event(&e));
    
    // Find event
    TEST_ASSERT_NOT_NULL(registry.find_event("event"));

    // We shouldn't be able to add an event that already exists
    TEST_ASSERT_FALSE(registry.add_event(&e));

    // We shouldn't be able to find an event that doesn't exist
    TEST_ASSERT_FALSE(registry.find_event("fake_event"));
}

void test_faults() {
    StateFieldRegistry registry;

    // Add fault to registry
    unsigned int control_cycle_count=1;
    Fault f("fault", 1, control_cycle_count);
    TEST_ASSERT_TRUE(registry.add_fault(&f));

    // Check that the fault and its writable fields were added to the registry
    TEST_ASSERT_NOT_NULL(registry.find_fault("fault"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("fault"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("fault.suppress"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("fault.override"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("fault.unsignal"));
    TEST_ASSERT_NOT_NULL(registry.find_writable_field("fault.persistence"));

    // We shouldn't be able to add a fault that already exists
    TEST_ASSERT_FALSE(registry.add_fault(&f));

    // We shouldn't be able to find a fault that doesn't exist
    TEST_ASSERT_FALSE(registry.find_fault("fake_fault"));
}

void test_state_field_registry() {
    UNITY_BEGIN();
    RUN_TEST(test_foo);
    RUN_TEST(test_events);
    RUN_TEST(test_faults);
    UNITY_END();
}

#ifdef DESKTOP
int main(int argc, char *argv[]) {
    test_state_field_registry();
    return 0;
}
#else
#include <Arduino.h>
void setup() {
    delay(10000);
    Serial.begin(9600);
    test_state_field_registry();
}

void loop() {}
#endif
