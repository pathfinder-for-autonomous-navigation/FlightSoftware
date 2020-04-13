#include <unity.h>
#include "../StateFieldRegistryMock.hpp"
#include <fsw/FCCode/PiksiFaultHandler.hpp>
#include <fsw/FCCode/piksi_mode_t.enum>
#include <fsw/FCCode/mission_state_t.enum>

unsigned int& cc_count = TimedControlTaskBase::control_cycle_count;

class TestFixture {
  public:
    StateFieldRegistryMock registry;

    std::shared_ptr<ReadableStateField<unsigned int>> piksi_state_fp;
    std::shared_ptr<WritableStateField<unsigned char>> mission_state_fp;
    std::shared_ptr<InternalStateField<unsigned int>> last_fix_time_ccno_fp;
    std::shared_ptr<InternalStateField<unsigned int>> enter_close_appr_time_fp;

    std::unique_ptr<PiksiFaultHandler> pfh;

    TestFixture() : registry() {
        piksi_state_fp = registry.create_internal_field<unsigned char>("piksi.state");
        mission_state_fp = registry.create_internal_field<unsigned int>("pan.state");
        last_fix_time_ccno_fp = registry.create_writable_field<bool>("piksi.last_fix_time_ccno");
        enter_close_appr_time_fp = registry.create_writable_field<bool>("piksi.last_fix_time_ccno");

        // Set initial conditions
        mission_state_fp->set(mission_state_t::manual);
        piksi_state_fp->set(piksi_mode_t::fixed_rtk);
        cc_count = 0;

        pfh = std::make_unique<PiksiFaultHandler>(registry);
    }
};

void test_task_initialization() {
    TestFixture tf;
    TEST_ASSERT_NOT_NULL(tf.piksi_state_fp);
    TEST_ASSERT_NOT_NULL(tf.mission_state_fp);
    TEST_ASSERT_NOT_NULL(tf.last_fix_time_ccno_fp);
    TEST_ASSERT_NOT_NULL(tf.enter_close_appr_time_fp);

    TEST_ASSERT_EQUAL(mission_state_t::manual, tf.mission_state_fp->get());
    TEST_ASSERT_EQUAL(piksi_mode_t::fixed_rtk, tf.piksi_state_fp->get());
    TEST_ASSERT_EQUAL(0, tf.cc_count);
}

void test_no_faults() {
    TestFixture tf;

}

int test_fault_handler() {
    UNITY_BEGIN();
    RUN_TEST(test_task_initialization);
    RUN_TEST(test_no_faults);
    return UNITY_END();
}

#ifdef DESKTOP
int main() {
    return test_fault_handler();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_fault_handler();
}
void loop() {}
#endif