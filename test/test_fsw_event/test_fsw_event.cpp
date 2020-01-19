#include "../../src/FCCode/Event.hpp"
#include "../../src/FCCode/EventStorage.hpp"
#include "../StateFieldRegistryMock.hpp"
#include <unity.h>

struct TestFixtureEvent {
  public:
    ReadableStateField<bool> data1_f;
    ReadableStateField<bool> data2_f;
    std::vector<ReadableStateFieldBase*> event_data;
    unsigned int control_cycle_count = 0;
    Event event;

    static char print_data[40];
    static const char* print_fn(const unsigned int ccno, std::vector<ReadableStateFieldBase*>& data) {
        memset(print_data, 0, 40);
        ReadableStateField<bool>* datafield1_f = static_cast<ReadableStateField<bool>*>(data[0]);
        ReadableStateField<bool>* datafield2_f = static_cast<ReadableStateField<bool>*>(data[1]);
        sprintf((char*)print_data,  "E: time: %d, data: %d, %d", ccno, datafield1_f->get(), datafield2_f->get());
        return print_data;
    }

    TestFixtureEvent() :
        data1_f("data1", Serializer<bool>()),
        data2_f("data2", Serializer<bool>()),
        event_data({&data1_f, &data2_f}),
        event("event", event_data, print_fn, control_cycle_count)
    {
        data1_f.set(false);
        data2_f.set(false);
    }
};

char TestFixtureEvent::print_data[40];

void test_single_event(TestFixtureEvent& tf) {
    // Test bitsize
    TEST_ASSERT_EQUAL(32 + 2, tf.event.bitsize());

    // Set values for test
    tf.control_cycle_count = 20;
    tf.data1_f.set(true);
    tf.data2_f.set(false);

    // Verify that upon serialization, the values are written into the event's bitset in the way
    // that we would expect
    tf.event.signal();
    const bit_array& ba = tf.event.get_bit_array();
    std::bitset<32> ccno;
    for(int i = 0; i < 32; i++) ccno[i] = ba[i];
    TEST_ASSERT_EQUAL(20, ccno.to_ulong());
    TEST_ASSERT_EQUAL(true, ba[32]);
    TEST_ASSERT_EQUAL(false, ba[33]);

    // Test that changes in the event values are picked up
    tf.data1_f.set(false);
    tf.data2_f.set(true);
    tf.event.signal();
    TEST_ASSERT_EQUAL(false, ba[32]);
    TEST_ASSERT_EQUAL(true, ba[33]);

    const char* print_result = tf.event.print();
    TEST_ASSERT_EQUAL_STRING("E: time: 20, data: 0, 1", print_result);
}

void test_event() {
    TestFixtureEvent tf;
    test_single_event(tf);
}

struct TestFixtureEventStorage : public TestFixtureEvent {
  public:
    EventStorage event_storage;
    StateFieldRegistryMock registry;

    TestFixtureEventStorage() :
        TestFixtureEvent(),
        event_storage("event", 3, event_data, print_fn, control_cycle_count)
    {
        event_storage.add_event_to_registry(registry);
    }
  protected:
    using TestFixtureEvent::event;
};

// Test that the event storage correctly manages pointers for event storage.
void test_event_storage() {
    TestFixtureEventStorage tf;
    // Three fields should have been created inside the state field registry.
    tf.registry.check_field_exists(
        tf.registry.find_readable_field_t<bool>("event.1"), "event.1");
    tf.registry.check_field_exists(
        tf.registry.find_readable_field_t<bool>("event.2"), "event.2");
    tf.registry.check_field_exists(
        tf.registry.find_readable_field_t<bool>("event.3"), "event.3");

    // Event storage should behave the same as an event.
    test_single_event(tf);
    test_single_event(tf);
    test_single_event(tf);
    test_single_event(tf);
}

#ifdef DESKTOP
int main() {
    UNITY_BEGIN();
    RUN_TEST(test_event);
    RUN_TEST(test_event_storage);
    return UNITY_END();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    UNITY_BEGIN();
    RUN_TEST(test_event);
    RUN_TEST(test_event_storage);
    UNITY_END();
}

void loop() {}
#endif
