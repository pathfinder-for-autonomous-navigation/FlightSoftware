#include "../StateFieldRegistryMock.hpp"

#include <fsw/FCCode/Event.hpp>
#include <fsw/FCCode/EventStorage.hpp>

#include <unity.h>

struct TestFixtureEvent
{
public:
    ReadableStateField<bool> data1_f;
    ReadableStateField<bool> data2_f;
    std::vector<ReadableStateFieldBase *> event_data;
    unsigned int control_cycle_count = 0;
    Event event;

    static char print_data[40];
    static const char *print_fn(const unsigned int ccno, std::vector<ReadableStateFieldBase *> &data)
    {
        memset(print_data, 0, 40);
        ReadableStateField<bool> *datafield1_f = static_cast<ReadableStateField<bool> *>(data[0]);
        ReadableStateField<bool> *datafield2_f = static_cast<ReadableStateField<bool> *>(data[1]);
        sprintf((char *)print_data, "E: time: %d, data: %d, %d", ccno, datafield1_f->get(), datafield2_f->get());
        return print_data;
    }

    TestFixtureEvent() : data1_f("data1", Serializer<bool>()),
                         data2_f("data2", Serializer<bool>()),
                         event_data({&data1_f, &data2_f}),
                         event("event", event_data, print_fn, control_cycle_count)
    {
        data1_f.set(false);
        data2_f.set(false);
    }
};

char TestFixtureEvent::print_data[40];

void test_single_event(TestFixtureEvent &tf, EventBase &event, unsigned int ccno)
{
    // Test bitsize
    TEST_ASSERT_EQUAL(32 + 2, tf.event.bitsize());

    // Set values for test
    tf.control_cycle_count = ccno;
    tf.data1_f.set(true);

    //std::cout << "first tf.data1_f " << tf.data1_f.get() << std::endl;
    //std::cout << "first ba[32] " << ba[32] << std::endl;

    tf.data2_f.set(false);

    // Verify that upon serialization, the values are written into the event's bitset in the way
    // that we would expect
    event.signal();
    bit_array &ba = const_cast<bit_array &>(event.get_bit_array());
    TEST_ASSERT_EQUAL(ccno, ba.to_uint());
    TEST_ASSERT_EQUAL(true, ba[32]);
    TEST_ASSERT_EQUAL(false, ba[33]);

    // Test that changes in the event values are picked up
    tf.data1_f.set(false);
    tf.data2_f.set(true);
    event.signal();
    ba = const_cast<bit_array &>(event.get_bit_array());
    event.next_event();
    TEST_ASSERT_EQUAL(false, ba[32]);
    TEST_ASSERT_EQUAL(true, ba[33]);

    // Test that the event is correctly printed when a print is requested.
    const char *print_result = event.print();
    const char *expected_fmt_string = "E: time: %d, data: 0, 1";
    char expected_string[100];
    memset(expected_string, 0, 100);
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
                                                       event_storage("event", num_events, event_data, print_fn, control_cycle_count)
    {
        event_storage.add_events_to_registry(registry);
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
        //printf("%d\n", i);
        test_single_event(tf, tf.event_storage, i);
        //TEST_ASSERT_EQUAL(i * 2, tf.event_storage.event_ptr);
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
