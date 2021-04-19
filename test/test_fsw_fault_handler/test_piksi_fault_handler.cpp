#include "../custom_assertions.hpp"
#include "../StateFieldRegistryMock.hpp"
#include <fsw/FCCode/PiksiFaultHandler.hpp>
#include <fsw/FCCode/piksi_mode_t.enum>
#include <fsw/FCCode/mission_state_t.enum>
#include "test_fault_handlers.hpp"

static unsigned int& cc_count = TimedControlTaskBase::control_cycle_count;

class TestFixture {
  public:
    StateFieldRegistryMock registry;

    // Fields used to create fault handler
    std::shared_ptr<ReadableStateField<unsigned char>> piksi_state_fp;
    std::shared_ptr<WritableStateField<unsigned char>> mission_state_fp;
    std::shared_ptr<InternalStateField<unsigned int>> enter_close_appr_ccno_fp;

    // Fields created by the fault handler
    WritableStateField<unsigned int>* no_cdgps_max_wait_fp;
    WritableStateField<unsigned int>* cdgps_delay_max_wait_fp;
    WritableStateField<bool>* fault_handler_enabled_fp;
    InternalStateField<unsigned int>* last_rtkfix_ccno_fp;

    std::unique_ptr<PiksiFaultHandler> pfh;

    void set_mission_state(mission_state_t state) {
        mission_state_fp->set(static_cast<unsigned char>(state));
    }
    void set_piksi_state(piksi_mode_t state) {
        piksi_state_fp->set(static_cast<unsigned int>(state));
    }

    TestFixture() : registry() {
        piksi_state_fp = registry.create_readable_field<unsigned char>("piksi.state");
        mission_state_fp = registry.create_writable_field<unsigned char>("pan.state");
        enter_close_appr_ccno_fp = registry.create_internal_field<unsigned int>("pan.enter_close_approach_ccno");

        // Set initial conditions
        set_mission_state(mission_state_t::manual);
        set_piksi_state(piksi_mode_t::fixed_rtk);
        cc_count = 0;

        pfh = std::make_unique<PiksiFaultHandler>(registry);

        no_cdgps_max_wait_fp = registry.find_writable_field_t<unsigned int>("piksi_fh.no_cdpgs_max_wait");
        cdgps_delay_max_wait_fp = registry.find_writable_field_t<unsigned int>("piksi_fh.cdpgs_delay_max_wait");
        fault_handler_enabled_fp = registry.find_writable_field_t<bool>("piksi_fh.enabled");
        last_rtkfix_ccno_fp = registry.find_internal_field_t<unsigned int>("piksi_fh.last_rtkfix_ccno");
    }
};

/**
 * Test that all the statefields have been created and are
 * initialized to the correct values.
 */
void test_task_initialization() {
    TestFixture tf;
    TEST_ASSERT_NOT_NULL(tf.piksi_state_fp);
    TEST_ASSERT_NOT_NULL(tf.mission_state_fp);
    TEST_ASSERT_NOT_NULL(tf.last_rtkfix_ccno_fp);
    TEST_ASSERT_NOT_NULL(tf.enter_close_appr_ccno_fp);
    TEST_ASSERT_NOT_NULL(tf.no_cdgps_max_wait_fp);
    TEST_ASSERT_NOT_NULL(tf.cdgps_delay_max_wait_fp);
    TEST_ASSERT_NOT_NULL(tf.fault_handler_enabled_fp);

    TEST_ASSERT_EQUAL(PAN::one_day_ccno, tf.no_cdgps_max_wait_fp->get());
    TEST_ASSERT_EQUAL(PAN::one_day_ccno/8, tf.cdgps_delay_max_wait_fp->get());
    TEST_ASSERT_EQUAL(true, tf.fault_handler_enabled_fp->get());
    TEST_ASSERT_EQUAL(mission_state_t::manual, tf.mission_state_fp->get());
    TEST_ASSERT_EQUAL(piksi_mode_t::fixed_rtk, tf.piksi_state_fp->get());
    TEST_ASSERT_EQUAL(0, cc_count);
}

static void test_no_faults() {
    TestFixture tf;
    fault_response_t response = tf.pfh->execute();
    TEST_ASSERT_EQUAL(fault_response_t::none, response);
}

/**
 * Test that the fault handler will recommend moving to standby if
 * the Piksi is dead.
 */
void test_dead_piksi() {
    TestFixture tf;
    tf.set_mission_state(mission_state_t::leader);
    tf.set_piksi_state(piksi_mode_t::crc_error);
    for(unsigned int i = 0; i < PAN::one_day_ccno/6; i++){
        tf.pfh->execute();
    }
    TEST_ASSERT_FALSE(tf.pfh->piksi_dead_fault_f.is_faulted());
    TEST_ASSERT_EQUAL(fault_response_t::standby, tf.pfh->execute());
    TEST_ASSERT_TRUE(tf.pfh->piksi_dead_fault_f.is_faulted());
}

/**
 * Test that the fault handler will recommend moving to standby if we 
 * have moved to a close approach state and haven't recieved any GPS for 
 * a designated maximum wait time since moving into the close approach state.
 */
void test_no_cdgps() {
    TestFixture tf;

    // The maximum time to wait for a GPS reading after entering close approach.
    // Default value is 24 hours.
    unsigned int no_gps_max_wait = tf.no_cdgps_max_wait_fp->get();

    // Get a GPS reading in manual state
    tf.last_rtkfix_ccno_fp->set(cc_count);
    tf.set_piksi_state(piksi_mode_t::no_fix);

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

/**
 * Test that the fault handler will recommend moving to standby if we 
 * recieve a GPS reading and move to a close approach state on the same
 * control cycle and have not recieved a subsequent GPS reading for a 
 * designated maximum wait time.
 */
void test_no_cdgps_2() {
    TestFixture tf;

    // The maximum time to wait for a GPS reading after entering close approach.
    // Default value is 24 hours.
    unsigned int no_gps_max_wait = tf.no_cdgps_max_wait_fp->get();

    // Get a GPS reading in manual state
    tf.last_rtkfix_ccno_fp->set(cc_count);
    tf.set_piksi_state(piksi_mode_t::no_fix);

    // Move to the close approach state
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

/**
 * Test that the fault handler will recommend moving to standby if we have been
 * recieving GPS readings while in a close approach state, but then haven't
 * recieved any more readings in a while for a designating maximum wait time.
 */
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
    tf.last_rtkfix_ccno_fp->set(cc_count);
    tf.set_piksi_state(piksi_mode_t::no_fix);

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

/**
 * Test that the fault handler will not return any fault responses while
 * the enable flag is turned off, even if a fault (a cdgps delay in this 
 * case) is triggered.
 */
void test_disable() {
    TestFixture tf;

    // Disable the fault handler
    tf.fault_handler_enabled_fp->set(false);

    // The maximum time to wait for a GPS reading in close approach.
    // Default value is 3 hours.
    unsigned int gps_delay_max_wait = tf.cdgps_delay_max_wait_fp->get();

    // Move to the close approach state
    tf.set_mission_state(mission_state_t::leader_close_approach);
    tf.enter_close_appr_ccno_fp->set(cc_count);

    // Wait a few control cycles and get a GPS reading
    cc_count = 10;
    tf.last_rtkfix_ccno_fp->set(cc_count);
    tf.set_piksi_state(piksi_mode_t::no_fix);

    // Let a little more than the maximum wait time pass since the last GPS reading
    cc_count += gps_delay_max_wait+1;

    // Check that no fault response is returned by the fault handler
    fault_response_t response = tf.pfh->execute();
    TEST_ASSERT_EQUAL(fault_response_t::none, response);
}

void test_piksi_fault_handler() {
    RUN_TEST(test_task_initialization);
    RUN_TEST(test_no_faults);
    RUN_TEST(test_dead_piksi);
    RUN_TEST(test_no_cdgps);
    RUN_TEST(test_no_cdgps_2);
    RUN_TEST(test_cdgps_delay);
    RUN_TEST(test_disable);
}
