#include "../StateFieldRegistryMock.hpp"

#include <fsw/FCCode/Event.hpp>
#include <fsw/FCCode/EventStorage.hpp>

#include <unity.h>

struct TestFixtureEvent
{
public:
    StateFieldRegistryMock registry;

    ReadableStateField<bool> data1_f;
    ReadableStateField<bool> data2_f;
    std::vector<ReadableStateFieldBase *> event_data;
    ReadableStateField<unsigned int> *control_cycle_count_ptr;
    Event event;
    size_t *event_ptr = nullptr;

    static char print_data[40];
    static const char *print_fn(const unsigned int ccno, std::vector<ReadableStateFieldBase *> &data)
    {
        memset(print_data, 0, 40);
        ReadableStateField<bool> *datafield1_f = static_cast<ReadableStateField<bool> *>(data[0]);
        ReadableStateField<bool> *datafield2_f = static_cast<ReadableStateField<bool> *>(data[1]);
        sprintf((char *)print_data, "E: time: %d, data: %d, %d", ccno, datafield1_f->get(), datafield2_f->get());
        return print_data;
    }

    TestFixtureEvent() : registry(),
                         data1_f("data1", Serializer<bool>()),
                         data2_f("data2", Serializer<bool>()),
                         event_data({&data1_f, &data2_f}),
                         event("event", event_data, print_fn)
    {
        data1_f.set(false);
        data2_f.set(false);
        control_cycle_count_ptr = registry.create_readable_field<unsigned int>("pan.cycle_no").get();
        event.ccno = control_cycle_count_ptr;
    }
};

char TestFixtureEvent::print_data[40];

void test_single_event(TestFixtureEvent &tf, EventBase &event, unsigned int ccno)
{
    // Test bitsize
    TEST_ASSERT_EQUAL(32 + 2, tf.event.bitsize());

    // Set values for test
    tf.control_cycle_count_ptr->set(ccno);
    tf.data1_f.set(true);
    tf.data2_f.set(false);

    //save pointer before signaling
    size_t event_ptr_prev;
    size_t event_ptr_curr;

    // Verify that upon serialization, the values are written into the event's bitset in the way
    // that we would expect
    if (tf.event_ptr != nullptr)
    {
        event_ptr_prev = *tf.event_ptr;
    }
    event.signal();
    if (tf.event_ptr != nullptr)
    {
        event_ptr_curr = *tf.event_ptr;
        *tf.event_ptr = event_ptr_prev;
    }
    bit_array &ba = const_cast<bit_array &>(event.get_bit_array());
    TEST_ASSERT_EQUAL(ccno, ba.to_uint());
    TEST_ASSERT_EQUAL(true, ba[32]);
    TEST_ASSERT_EQUAL(false, ba[33]);
    if (tf.event_ptr != nullptr)
    {
        *tf.event_ptr = event_ptr_curr;
    }

    // Test that changes in the event values are picked up
    tf.data1_f.set(false);
    tf.data2_f.set(true);
    if (tf.event_ptr != nullptr)
    {
        event_ptr_prev = *tf.event_ptr;
    }
    event.signal();
    if (tf.event_ptr != nullptr)
    {
        event_ptr_curr = *tf.event_ptr;
        *tf.event_ptr = event_ptr_prev;
    }
    ba = const_cast<bit_array &>(event.get_bit_array());
    TEST_ASSERT_EQUAL(false, ba[32]);
    TEST_ASSERT_EQUAL(true, ba[33]);
    if (tf.event_ptr != nullptr)
    {
        *tf.event_ptr = event_ptr_curr;
    }

    // Test that the event is correctly printed when a print is requested.
    const char *print_result = event.print();
    const char *expected_fmt_string = "E: time: %d, data: 0, 1";
    char expected_string[100];
    sprintf(expected_string, expected_fmt_string, ccno);
    TEST_ASSERT_EQUAL_STRING(expected_string, print_result);
}

void test_event()
{
    TestFixtureEvent tf;
    test_single_event(tf, tf.event, 20);
}

struct TestFixtureEventStorage : public TestFixtureEvent
{
public:
    EventStorage event_storage;
    StateFieldRegistryMock registry;

    TestFixtureEventStorage(unsigned int num_events) : TestFixtureEvent(),
                                                       event_storage("event", num_events, event_data, print_fn)
    {
        event_storage.add_events_to_registry(registry);
        event_ptr = &event_storage.event_ptr;
    }

protected:
    using TestFixtureEvent::event;
};

// Test that the event storage correctly manages pointers for event storage.
void test_event_storage()
{
    TestFixtureEventStorage tf(99);
    // Fields should have been created inside the state field registry.
    tf.registry.find_readable_field("event.1");
    tf.registry.find_readable_field("event.2");
    tf.registry.find_readable_field("event.3");
    // Test that a field was added to the registry for every single sub-event.
    TEST_ASSERT_EQUAL(99, tf.registry.readable_fields.size());

    // Event storage should behave the same as an event.
    for (int i = 0; i < 200; i++)
    {
        TEST_ASSERT_EQUAL((2 * i) % 99, *tf.event_ptr); //event signaled twice per single event, storage size 99
        test_single_event(tf, tf.event_storage, i);
    }
}

#ifdef DESKTOP
int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_event);
    RUN_TEST(test_event_storage);
    return UNITY_END();
}
#else
#include <Arduino.h>
void setup()
{
    delay(2000);
    Serial.begin(9600);
    UNITY_BEGIN();
    RUN_TEST(test_event);
    RUN_TEST(test_event_storage);
    UNITY_END();
}

void loop() {}
#endif
