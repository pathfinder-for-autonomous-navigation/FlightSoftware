#include "../StateFieldRegistryMock.hpp"
#include <fsw/FCCode/ControlTask.hpp>

#include <unity.h>

class DummyControlTask : public ControlTask<void> {
  public:
    DummyControlTask(StateFieldRegistry& registry) : ControlTask<void>(registry) {}
    void execute() { 
      DummyControlTask::x = 3;
    }
    int x = 2;
};

void test_task_initialization() {
    StateFieldRegistryMock registry;
    DummyControlTask task(registry);
}

void test_task_execute() {
    StateFieldRegistryMock registry;
    DummyControlTask task(registry);
    TEST_ASSERT_EQUAL(2, task.x);
    task.execute();
    TEST_ASSERT_EQUAL(3, task.x);
}

// Construction of dummy event and fault objects, used in the find and add tests below.
// This construction also implicitly tests that constructing events and faults statically
// does not cause a segfault or other undesirable behaviors.
const char* event_print_fn(const unsigned int cc_count,
    std::vector<ReadableStateFieldBase*>& fields)
{
    static const char* x = "";
    return x;
}

static std::vector<ReadableStateFieldBase*> event_fields_list {};
static Event event("event", event_fields_list, event_print_fn);
static Fault fault("fault", 1);

void test_task_find() {
    StateFieldRegistryMock registry;
    DummyControlTask task(registry);

    // Finding internal, readable, and writable fields should work.
    auto internal_fp = registry.create_internal_field<bool>("field1");
    auto readable_fp = registry.create_readable_field<bool>("field2");
    auto writable_fp = registry.create_writable_field<bool>("field3");
    TEST_ASSERT_NOT_NULL(task.find_internal_field<bool>("field1", __FILE__, __LINE__));
    TEST_ASSERT_NOT_NULL(task.find_readable_field<bool>("field2", __FILE__, __LINE__));
    TEST_ASSERT_NOT_NULL(task.find_writable_field<bool>("field3", __FILE__, __LINE__));

    // Finding a fault should work.
    registry.add_fault(&fault);
    TEST_ASSERT_NOT_NULL(task.find_fault("fault", __FILE__, __LINE__));

    // Finding an event should work.
    registry.add_event(&event);
    TEST_ASSERT_NOT_NULL(task.find_event("event", __FILE__, __LINE__));
}

void test_task_add() {
    StateFieldRegistryMock registry;
    DummyControlTask task(registry);

    // Adding an internal field should work.
    InternalStateField<bool> field1("field1");
    task.add_internal_field(field1);
    TEST_ASSERT_NOT_NULL(registry.find_internal_field_t<bool>("field1"));

    // Adding a readable field should work.
    ReadableStateField<bool> field2("field2", Serializer<bool>());
    task.add_readable_field(field2);
    TEST_ASSERT_NOT_NULL(registry.find_readable_field_t<bool>("field2"));

    // Adding a writable field should work.
    WritableStateField<bool> field3("field3", Serializer<bool>());
    task.add_writable_field(field3);
    TEST_ASSERT_NOT_NULL(registry.find_writable_field_t<bool>("field3"));

    // Adding a fault should work
    task.add_fault(fault);
    TEST_ASSERT_NOT_NULL(registry.find_fault_t("fault"));

    // Adding an event should work
    task.add_event(event);
    TEST_ASSERT_NOT_NULL(registry.find_event_t("event"));
}

int test_control_task() {
    UNITY_BEGIN();
    RUN_TEST(test_task_initialization);
    RUN_TEST(test_task_execute);
    RUN_TEST(test_task_find);
    RUN_TEST(test_task_add);
    return UNITY_END();
}

#ifdef DESKTOP
int main() {
    return test_control_task();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_control_task();
}

void loop() {}
#endif
