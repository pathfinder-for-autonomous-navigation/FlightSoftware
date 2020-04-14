#include <unity.h>
#include "../StateFieldRegistryMock.hpp"
#include <fsw/FCCode/PiksiFaultHandler.hpp>
#include <fsw/FCCode/piksi_mode_t.enum>
#include <fsw/FCCode/mission_state_t.enum>
#include <iostream>

unsigned int& cc_count = TimedControlTaskBase::control_cycle_count;

class TestFixture {
  public:
    StateFieldRegistryMock registry;

    // Field used to create fault handler
    std::shared_ptr<ReadableStateField<unsigned int>> piksi_state_fp;
    std::shared_ptr<WritableStateField<unsigned char>> mission_state_fp;
    std::shared_ptr<InternalStateField<unsigned int>> last_fix_time_ccno_fp;
    std::shared_ptr<InternalStateField<unsigned int>> enter_close_appr_ccno_fp;

    // Fields created by the fault handler
    WritableStateField<unsigned int>* no_cdgps_max_wait_fp;
    WritableStateField<unsigned int>* cdgps_delay_max_wait_fp;

    std::unique_ptr<PiksiFaultHandler> pfh;

    void set_mission_state(mission_state_t state) {
        mission_state_fp->set(static_cast<unsigned char>(state));
    }
    void set_piksi_state(piksi_mode_t state) {
        piksi_state_fp->set(static_cast<unsigned int>(state));
    }

    TestFixture() : registry() {
        piksi_state_fp = registry.create_readable_field<unsigned int>("piksi.state");
        mission_state_fp = registry.create_writable_field<unsigned char>("pan.state");
        last_fix_time_ccno_fp = registry.create_internal_field<unsigned int>("piksi.last_fix_time_ccno");
        enter_close_appr_ccno_fp = registry.create_internal_field<unsigned int>("pan.enter_close_approach_ccno");

        // Set initial conditions
        set_mission_state(mission_state_t::manual);
        set_piksi_state(piksi_mode_t::fixed_rtk);
        cc_count = 0;

        pfh = std::make_unique<PiksiFaultHandler>(registry);

        no_cdgps_max_wait_fp = registry.find_writable_field_t<unsigned int>("piksi_fh.no_cdpgs_max_wait");
        cdgps_delay_max_wait_fp = registry.find_writable_field_t<unsigned int>("piksi_fh.cdpgs_delay_max_wait");
    }
};

void test_task_initialization() {
    TestFixture tf;
    TEST_ASSERT_NOT_NULL(tf.piksi_state_fp);
    TEST_ASSERT_NOT_NULL(tf.mission_state_fp);
    TEST_ASSERT_NOT_NULL(tf.last_fix_time_ccno_fp);
    TEST_ASSERT_NOT_NULL(tf.enter_close_appr_ccno_fp);
    TEST_ASSERT_NOT_NULL(tf.no_cdgps_max_wait_fp);
    TEST_ASSERT_NOT_NULL(tf.cdgps_delay_max_wait_fp);

    TEST_ASSERT_EQUAL(PAN::one_day_ccno, tf.no_cdgps_max_wait_fp->get());
    TEST_ASSERT_EQUAL(PAN::one_day_ccno/8, tf.cdgps_delay_max_wait_fp->get());
    TEST_ASSERT_EQUAL(mission_state_t::manual, tf.mission_state_fp->get());
    TEST_ASSERT_EQUAL(piksi_mode_t::fixed_rtk, tf.piksi_state_fp->get());
    TEST_ASSERT_EQUAL(0, cc_count);
}

void test_no_faults() {
    TestFixture tf;
    fault_response_t response = tf.pfh->execute();
    TEST_ASSERT_EQUAL(fault_response_t::none, response);
}

void test_dead_piksi() {
    TestFixture tf;
    tf.set_piksi_state(piksi_mode_t::dead);
    fault_response_t response = tf.pfh->execute();
    TEST_ASSERT_EQUAL(fault_response_t::standby, response);
}

void test_no_cdgps() {
    TestFixture tf;

    // The maximum time to wait for a GPS reading after entering close approach.
    // Default value is 24 hours.
    unsigned int no_gps_max_wait = tf.no_cdgps_max_wait_fp->get();

    // Get a GPS reading in manual state
    tf.last_fix_time_ccno_fp->set(cc_count);

    // Wait a few control cycles and move to the close approach state
    cc_count+=30;
    tf.set_mission_state(mission_state_t::leader_close_approach);
    tf.enter_close_appr_ccno_fp->set(cc_count);

    // Let a the maximum wait time pass since the moving to close approach
    cc_count += no_gps_max_wait;

    // Check that no fault response is returned by the fault handler
    fault_response_t response = tf.pfh->execute();
    TEST_ASSERT_EQUAL(fault_response_t::none, response);

    // Let a little more than the maximum wait time pass since moving to close approach
    cc_count += 1;

    // Check that the fault handler recommends moving to standby
    response = tf.pfh->execute();
    TEST_ASSERT_EQUAL(fault_response_t::standby, response);
}

void test_cdgps_delay() {
    TestFixture tf;

    // The maximum time to wait for a GPS reading in close approach.
    // Default value is 3 hours.
    unsigned int gps_delay_max_wait = tf.cdgps_delay_max_wait_fp->get();

    // Move to the close approach state
    tf.set_mission_state(mission_state_t::leader_close_approach);
    tf.enter_close_appr_ccno_fp->set(cc_count);

    // Wait a few control cycles and get a GPS reading
    cc_count = 10;
    tf.last_fix_time_ccno_fp->set(cc_count);

    // Let the maximum wait time pass since the last GPS reading
    cc_count += gps_delay_max_wait;

    // Check that no fault response is returned by the fault handler
    fault_response_t response = tf.pfh->execute();
    TEST_ASSERT_EQUAL(fault_response_t::none, response);

    // Let a little more than the maximum wait time pass since the last GPS reading
    cc_count += 1;

    // Check that the fault handler recommends moving to standby
    response = tf.pfh->execute();
    TEST_ASSERT_EQUAL(fault_response_t::standby, response);
}

int test_fault_handler() {
    UNITY_BEGIN();
    RUN_TEST(test_task_initialization);
    RUN_TEST(test_no_faults);
    RUN_TEST(test_dead_piksi);
    RUN_TEST(test_no_cdgps);
    RUN_TEST(test_cdgps_delay);
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