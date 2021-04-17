#include "../custom_assertions.hpp"
#include "../StateFieldRegistryMock.hpp"
#include <fsw/FCCode/QuakeFaultHandler.hpp>
#include <fsw/FCCode/radio_state_t.enum>
#include "test_fault_handlers.hpp"

static unsigned int one_day_ccno = PAN::one_day_ccno;
static unsigned int &cc_count = TimedControlTaskBase::control_cycle_count;

class TestFixtureQFH
{
public:
    StateFieldRegistryMock registry;

    std::shared_ptr<ReadableStateField<unsigned char>> radio_state_fp;
    std::shared_ptr<ReadableStateField<unsigned int>> last_checkin_cycle_fp;
    std::shared_ptr<WritableStateField<bool>> radio_power_cycle_fp;

    std::unique_ptr<QuakeFaultHandler> qfh;

    WritableStateField<unsigned char> *qfh_state;
    WritableStateField<bool>* fault_handler_enabled_fp;

    void disable_radio()
    {
        radio_state_fp->set(static_cast<unsigned char>(radio_state_t::disabled));
    }
    void enable_radio()
    {
        radio_state_fp->set(static_cast<unsigned char>(radio_state_t::config));
    }

    TestFixtureQFH(qfh_state_t initial_state = qfh_state_t::unfaulted)
    {
        radio_state_fp = registry.create_readable_field<unsigned char>("radio.state");
        last_checkin_cycle_fp = registry.create_readable_field<unsigned int>("radio.last_comms_ccno");
        radio_power_cycle_fp = registry.create_writable_field<bool>("gomspace.power_cycle_output3_cmd");

        // Set initial conditions
        enable_radio();
        last_checkin_cycle_fp->set(0);
        radio_power_cycle_fp->set(false);
        cc_count = 0;

        qfh = std::make_unique<QuakeFaultHandler>(registry);

        qfh_state = registry.find_writable_field_t<unsigned char>("qfh.state");
        set(initial_state);
        fault_handler_enabled_fp = registry.find_writable_field_t<bool>("qfh.enabled");
    }

    void set(qfh_state_t state) { qfh->cur_state.set(static_cast<unsigned char>(state)); }
    void set(unsigned int state) { qfh->cur_state.set(state); }

    // Below are getter and setter methods for the test harness, listed in order of
    // increasing invasiveness/complexity.

    // Verify that the radio was powercycled (or not).
    void check_powercycled() { TEST_ASSERT_TRUE(radio_power_cycle_fp->get()); }
    void check_not_powercycled() { TEST_ASSERT_FALSE(radio_power_cycle_fp->get()); }

    // Get and set the control cycle # on which the current fault state was entered.
    unsigned int get_cur_state_entry_ccno() const { return qfh->cur_state_entry_ccno; }
    void set_cur_state_entry_ccno(unsigned int ccno) { qfh->cur_state_entry_ccno = ccno; }

    /**
     * @brief Manually force a fault transition, perhaps beyond the existing arrows in the DFA.
     * 
     * @param state Desired state of fault machine.
     */
    void transition_to(qfh_state_t state) { qfh->transition_to(state); }

    /**
     * @brief Step forward one control cycle and check if the resultant state of the fault machine
     * and its recommended mission state output are what we would expect.
     * 
     * @param expected_mission_state 
     * @param expected_fault_state 
     */
    void step_and_expect(fault_response_t expected_response, qfh_state_t expected_fault_state)
    {
        fault_response_t response = qfh->execute();
        TEST_ASSERT_EQUAL(expected_response, response);
        TEST_ASSERT_EQUAL(static_cast<unsigned char>(expected_fault_state), qfh->cur_state.get());
        cc_count++;
    }

    /**
     * @brief Step the machine forward once, and verify that the state of the machine returns to
     * "unfaulted" if the radio is disabled. 
     * 
     * This is a useful helper function since there are many arrows from fault states to the
     * "unfaulted" states that are conditional on the radio being disabled.
     */
    void check_state_returns_to_unfaulted_if_radio_disabled()
    {
        disable_radio();
        step_and_expect(fault_response_t::none, qfh_state_t::unfaulted);
    }

    /**
     * @brief Step the machine forward once, and verify that the state of the machine returns to
     * "unfaulted" if we happen to receive comms within the duration of the state.
     * 
     * This is a useful helper function since every faulted state has an arrow back to the "unfaulted"
     * state if we successfully received comms.
     * 
     * @param state_duration in number of control cycles.
     */
    void check_state_returns_to_unfaulted_if_comms_recvd(const unsigned int state_duration)
    {
        cc_count = state_duration - 1;
        step_and_expect(fault_response_t::none, qfh_state_t::unfaulted);

        // Cleanup
        cc_count = 0;
    }
};

void test_qfh_initialization()
{
    TestFixtureQFH tf{qfh_state_t::unfaulted};
    TEST_ASSERT_NOT_NULL(tf.qfh_state);
    TEST_ASSERT_NOT_NULL(tf.fault_handler_enabled_fp);
    TEST_ASSERT_EQUAL(static_cast<unsigned char>(qfh_state_t::unfaulted), tf.qfh_state->get());
    TEST_ASSERT_EQUAL(true, tf.fault_handler_enabled_fp->get());
}

void test_qfh_transition()
{
    // Open-box test.
    //
    // The Quake fault handler should cleanly reset the time
    // at which the most recent state was entered into upon
    // a state transition.
    TestFixtureQFH tf{qfh_state_t::unfaulted};
    tf.step_and_expect(fault_response_t::none, qfh_state_t::unfaulted);
    tf.step_and_expect(fault_response_t::none, qfh_state_t::unfaulted);
    tf.step_and_expect(fault_response_t::none, qfh_state_t::unfaulted);
    TEST_ASSERT_EQUAL(0, tf.get_cur_state_entry_ccno());
    tf.transition_to(qfh_state_t::forced_standby);
    TEST_ASSERT_EQUAL(3, tf.get_cur_state_entry_ccno());
}

void test_qfh_unfaulted()
{
    // If the radio is disabled, it doesn't matter how
    // long we've been without comms. There should be no
    // transition to a faulted state.
    {
        // Set initial conditions
        TestFixtureQFH tf{qfh_state_t::unfaulted};
        cc_count = one_day_ccno;
        tf.set_cur_state_entry_ccno(one_day_ccno);

        // Disable radio within the "24 hour" period of this state.
        // Verify that the state machine goes back to "unfaulted".
        tf.disable_radio();
        cc_count = 2 * one_day_ccno - 1;
        tf.step_and_expect(fault_response_t::none, qfh_state_t::unfaulted);
    }

    // If the radio is enabled, ensure that there's a transition
    // to the forced standby state after 24 hours.
    {
        TestFixtureQFH tf{qfh_state_t::unfaulted};
        tf.enable_radio();
        cc_count = one_day_ccno - 1;
        tf.step_and_expect(fault_response_t::none, qfh_state_t::unfaulted);
        tf.step_and_expect(fault_response_t::standby, qfh_state_t::forced_standby);
    }
}

void test_qfh_forced_standby()
{
    // If it's been more than 24 hours since comms, the fault handler
    // should cause a power cycle and move to the powercycle_1 state.
    //
    // We'll first test that one cycle less of 24 hours of no comms doesn't
    // cause a transition, and then cycling one more time causes a transition
    // to powercycle_1.
    {
        //Test if in wait, should power cycle as stated above
        TestFixtureQFH tf{qfh_state_t::forced_standby};
        tf.radio_state_fp->set(static_cast<unsigned char>(radio_state_t::wait));
        tf.set_cur_state_entry_ccno(one_day_ccno);
        cc_count = 2 * one_day_ccno - 1;
        tf.step_and_expect(fault_response_t::standby, qfh_state_t::forced_standby);
        tf.step_and_expect(fault_response_t::standby, qfh_state_t::powercycle_1);
        tf.check_powercycled();
    }

    // If the radio is disabled the state should return to unfaulted immediately.
    {
        TestFixtureQFH tf{qfh_state_t::forced_standby};
        tf.check_state_returns_to_unfaulted_if_radio_disabled();
    }

    // If we've (recently) received comms within 24 hours since the state transition, the state
    // should return to unfaulted immediately.
    {
        TestFixtureQFH tf{qfh_state_t::forced_standby};
        tf.check_state_returns_to_unfaulted_if_comms_recvd(one_day_ccno);
    }
}

void test_qfh_powercycle(qfh_state_t cur_state, qfh_state_t next_state)
{
    // If it's been more than 8 hours since comms, the fault handler
    // should cause a power cycle and move to the powercycle_2 state.
    //
    // We'll first test that one cycle less of 8 hours of no comms doesn't
    // cause a transition, and then cycling one more time causes a transition
    // to powercycle_2.
    {
        //Test if in wait, should power cycle as stated above
        TestFixtureQFH tf{cur_state};
        tf.set_cur_state_entry_ccno(one_day_ccno);
        cc_count = one_day_ccno + one_day_ccno / 3 - 1;
        tf.radio_state_fp->set(static_cast<unsigned char>(radio_state_t::wait));
        tf.step_and_expect(fault_response_t::standby, cur_state);
        tf.step_and_expect(fault_response_t::standby, next_state);
        if (next_state != qfh_state_t::safehold)
            tf.check_powercycled();
    }

    // If the radio is disabled the state should return to unfaulted immediately.
    {
        TestFixtureQFH tf{qfh_state_t::forced_standby};
        tf.check_state_returns_to_unfaulted_if_radio_disabled();
    }

    // If we've (recently) received comms within 8 hours since the state transition, the state
    // should return to unfaulted immediately.
    {
        TestFixtureQFH tf{cur_state};
        tf.check_state_returns_to_unfaulted_if_comms_recvd(one_day_ccno / 3);
    }
}

void test_qfh_powercycle_1() { test_qfh_powercycle(qfh_state_t::powercycle_1, qfh_state_t::powercycle_2); };
void test_qfh_powercycle_2() { test_qfh_powercycle(qfh_state_t::powercycle_2, qfh_state_t::powercycle_3); };
void test_qfh_powercycle_3() { test_qfh_powercycle(qfh_state_t::powercycle_3, qfh_state_t::safehold); };

void test_qfh_safehold()
{
    // If the radio is disabled the state should return to unfaulted immediately.
    {
        TestFixtureQFH tf{qfh_state_t::safehold};
        tf.check_state_returns_to_unfaulted_if_radio_disabled();
    }

    // If we've (recently) received comms within 24 hours since the state transition, the state
    // should return to unfaulted immediately.
    {
        TestFixtureQFH tf{qfh_state_t::safehold};
        tf.check_state_returns_to_unfaulted_if_comms_recvd(one_day_ccno);
    }

    // Otherwise, the machine should suggest safe hold as the recommended mission state.
    {
        TestFixtureQFH tf{qfh_state_t::safehold};
        tf.set_cur_state_entry_ccno(one_day_ccno);
        cc_count = one_day_ccno;

        tf.step_and_expect(fault_response_t::safehold, qfh_state_t::safehold);
        cc_count += one_day_ccno / 2;
        tf.step_and_expect(fault_response_t::safehold, qfh_state_t::safehold);
        cc_count += one_day_ccno / 2 - 1;
        tf.step_and_expect(fault_response_t::safehold, qfh_state_t::safehold);
        tf.step_and_expect(fault_response_t::safehold, qfh_state_t::safehold);
        tf.step_and_expect(fault_response_t::safehold, qfh_state_t::safehold);
        tf.step_and_expect(fault_response_t::safehold, qfh_state_t::safehold);
    }
}

// If the state value gets set to something undefined, it gets kicked back
// to unfaulted and the recommended mission state is "manual."
void test_qfh_undefined_state()
{
    TestFixtureQFH tf;
    tf.set(100);
    tf.step_and_expect(fault_response_t::none, qfh_state_t::unfaulted);
}

/**
 * Test that the fault handler will not return any fault responses while
 * the enable flag is turned off, even if a fault (a day of no comms in this 
 * case) is triggered.
 */
void test_qfh_disable() {
    // If the qfh is disabled, it doesn't matter how
    // long we've been without comms. There should be no
    // transition to a faulted state.
    {
        // Set initial conditions
        TestFixtureQFH tf{qfh_state_t::unfaulted};
        cc_count = one_day_ccno;
        tf.set_cur_state_entry_ccno(one_day_ccno);

        // Disable radio within the "24 hour" period of this state.
        // Verify that the state machine goes back to "unfaulted".
        // Disable the fault handler
        tf.fault_handler_enabled_fp->set(false);
        cc_count = 2 * one_day_ccno - 1;
        tf.step_and_expect(fault_response_t::none, qfh_state_t::unfaulted);
    }

    // If the qfh is enabled, ensure that there's a transition
    // to the forced standby state after 24 hours.
    {
        TestFixtureQFH tf{qfh_state_t::unfaulted};
        tf.fault_handler_enabled_fp->set(true);
        cc_count = one_day_ccno - 1;
        tf.step_and_expect(fault_response_t::none, qfh_state_t::unfaulted);
        tf.step_and_expect(fault_response_t::standby, qfh_state_t::forced_standby);
    }
}

void test_quake_fault_handler()
{
    RUN_TEST(test_qfh_initialization);
    RUN_TEST(test_qfh_unfaulted);
    RUN_TEST(test_qfh_forced_standby);
    RUN_TEST(test_qfh_powercycle_1);
    RUN_TEST(test_qfh_powercycle_2);
    RUN_TEST(test_qfh_powercycle_3);
    RUN_TEST(test_qfh_safehold);
    RUN_TEST(test_qfh_undefined_state);
    RUN_TEST(test_qfh_disable);
}
