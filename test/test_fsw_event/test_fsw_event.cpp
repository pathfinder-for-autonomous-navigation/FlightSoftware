#include "../../src/FCCode/Event.hpp"
#include "../StateFieldRegistryMock.hpp"
#include <unity.h>

struct TestFixture {
  public:
    ReadableStateField<bool> data1_f;
    ReadableStateField<bool> data2_f;
    std::vector<ReadableStateFieldBase*> event_data;
    unsigned int control_cycle_count = 0;
    Event event;

    char print_data[10];
    static const char* print_fn(std::vector<ReadableStateFieldBase*>& data) {
        memset(print_data, 0, 10);
        ReadableStateField<bool>* datafield1_f = static_cast<ReadableStateField<bool>*>(data[0]);
        ReadableStateField<bool>* datafield2_f = static_cast<ReadableStateField<bool>*>(data[1]);
        sprintf((char*)print_data,  "E: %d, %d", datafield1_f->get(), datafield2_f->get());
        return print_data;
    }

    TestFixture() :
        data1_f("data1", Serializer<bool>()),
        data2_f("data2", Serializer<bool>()),
        event_data({&data1_f, &data2_f}),
        event("event", event_data, print_fn, control_cycle_count) {}
};

void test_event() {
    TestFixture tf;
}

#ifdef DESKTOP
int main() {
    UNITY_BEGIN();
    RUN_TEST(test_event);
    return UNITY_END();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    UNITY_BEGIN();
    RUN_TEST(test_event);
    UNITY_END();
}

void loop() {}
#endif
