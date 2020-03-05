#include <fsw/FCCode/PropMonitor.hpp>
#include "../StateFieldRegistryMock.hpp"

struct TestFixture {
    StateFieldRegistryMock r;
    std::shared_ptr<PropMonitor> prop_monitor;

    ReadableStateField<temperature_t>* tank1_temp;
    ReadableStateField<temperature_t>* tank2_temp;
    ReadableStateField<unsigned int>* tank2_pressure;

    TestFixture() : r() {
        prop_monitor = std::make_shared<PropMonitor>(r, 0);

        tank1_temp = r.find_readable_field_t<temperature_t>("prop.tank1.temp");
        tank2_temp = r.find_readable_field_t<temperature_t>("prop.tank1.temp");
        tank2_pressure = r.find_readable_field_t<unsigned int>("prop.tank2.pressure");
    }
};

void test_initialization() {
    TestFixture tf;
    
    TEST_ASSERT_NOT_NULL(tf.tank1_temp);
    TEST_ASSERT_NOT_NULL(tf.tank2_temp);
    TEST_ASSERT_NOT_NULL(tf.tank2_pressure);
    TEST_ASSERT_EQUAL(0, tf.tank1_temp->get());
    TEST_ASSERT_EQUAL(0, tf.tank2_temp->get());
    TEST_ASSERT_EQUAL(0, tf.tank2_pressure->get());
}

void test_execute() {
    TestFixture tf;
    tf.prop_monitor.execute();

    #ifdef DESKTOP
        TEST_ASSERT_EQUAL(Tank::min_temp, tf.tank2_temp.get());
        TEST_ASSERT_EQUAL(Tank::min_temp, tf.tank2_temp.get());
        TEST_ASSERT_EQUAL(_Tank2::high_gain_offset, tf.tank2_pressure->get());
    #endif
    // Otherwise, use the test_prop_regression function below to verify that
    // the correct values are found.
}

void test_prop_regression() {
    TestFixture tf;

    #ifndef DESKTOP
        // TODO
    #endif
}

void test_prop_monitor() {
    RUN_TEST(test_initialization);
    RUN_TEST(test_execute);
    RUN_TEST(test_prop_regression);
}

#ifdef DESKTOP
int main() {
    UNITY_BEGIN();
    test_prop_monitor();
    return UNITY_END();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    UNITY_BEGIN();
    test_prop_monitor();
    UNITY_END();
}

void loop() {}
#endif
