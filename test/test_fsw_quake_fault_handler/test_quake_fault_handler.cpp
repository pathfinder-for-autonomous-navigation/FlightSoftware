#include <unity.h>
#include "../StateFieldRegistryMock.hpp"
#include <fsw/FCCode/QuakeFaultHandler.hpp>
#include <fsw/FCCode/radio_state_t.enum>

using fault_checker_state_t = QuakeFaultHandler::fault_checker_state_t;
unsigned int& control_cycle_count = TimedControlTaskBase::control_cycle_count;

class TestFixture {
  public:
    StateFieldRegistryMock registry;

    std::shared_ptr<InternalStateField<unsigned char>> radio_state_fp;
    std::shared_ptr<InternalStateField<unsigned int>> last_checkin_cycle_fp;
    std::shared_ptr<WritableStateField<bool>> radio_power_cycle_fp;

    std::unique_ptr<QuakeFaultHandler> qfh;

    void disable_radio() {
        radio_state_fp->set(static_cast<unsigned char>(radio_state_t::disabled));
    }
    void enable_radio() {
        radio_state_fp->set(static_cast<unsigned char>(radio_state_t::wait));
    }

    TestFixture(fault_checker_state_t initial_state = fault_checker_state_t::unfaulted) {
        radio_state_fp = registry.create_internal_field<unsigned char>("radio.state");
        last_checkin_cycle_fp = registry.create_internal_field<unsigned int>("radio.last_comms_ccno");
        radio_power_cycle_fp = registry.create_writable_field<bool>("gomspace.power_cycle_output1_cmd");

        // Set initial conditions
        enable_radio();
        last_checkin_cycle_fp->set(0);
        radio_power_cycle_fp->set(false);
        control_cycle_count = 0;

        qfh = std::make_unique<QuakeFaultHandler>(registry);
        qfh->cur_state = initial_state;
    }

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
    void transition_to(fault_checker_state_t state) { qfh->transition_to(state); }

    /**
     * @brief Step forward one control cycle and check if the resultant state of the fault machine
     * and its recommended mission state output are what we would expect.
     * 
     * @param expected_mission_state 
     * @param expected_fault_state 
     */
    void step_and_expect(mission_state_t expected_mission_state, fault_checker_state_t expected_fault_state) {
        mission_state_t state = qfh->execute();
        TEST_ASSERT_EQUAL(state, expected_mission_state);
        TEST_ASSERT_EQUAL(expected_fault_state, qfh->cur_state);
        control_cycle_count++;
    }

    /**
     * @brief Step the machine forward once, and verify that the state of the machine returns to
     * "unfaulted" if the radio is disabled. 
     * 
     * This is a useful helper function since there are many arrows from fault states to the
     * "unfaulted" states that are conditional on the radio being disabled.
     */
    void check_state_returns_to_unfaulted_if_radio_disabled() {
        disable_radio();
        step_and_expect(mission_state_t::manual, fault_checker_state_t::unfaulted);
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
    void check_state_returns_to_unfaulted_if_comms_recvd(const unsigned int state_duration) {
        control_cycle_count = state_duration - 1;
        step_and_expect(mission_state_t::manual, fault_checker_state_t::unfaulted);

        // Cleanup
        control_cycle_count = 0;
    }
};

/**
 * @brief Unit test the mocked fault checker utility so that we know
 * it'll work for Mission Manager.
 */
void test_qfhmock() {
    StateFieldRegistryMock r;
    QuakeFaultHandlerMock qfh(r);
    qfh.set(mission_state_t::follower);
    TEST_ASSERT_EQUAL(mission_state_t::follower, qfh.get());
    TEST_ASSERT_EQUAL(mission_state_t::follower, qfh.execute());
}

void test_qfh_initialization() { TestFixture tf; }

void test_qfh_transition() {
    // Open-box test.
    //
    // The Quake fault handler should cleanly reset the time
    // at which the most recent state was entered into upon
    // a state transition.
    TestFixture tf{fault_checker_state_t::unfaulted};
    tf.step_and_expect(mission_state_t::manual, fault_checker_state_t::unfaulted);
    tf.step_and_expect(mission_state_t::manual, fault_checker_state_t::unfaulted);
    tf.step_and_expect(mission_state_t::manual, fault_checker_state_t::unfaulted);
    TEST_ASSERT_EQUAL(0, tf.get_cur_state_entry_ccno());
    tf.transition_to(fault_checker_state_t::forced_standby);
    TEST_ASSERT_EQUAL(3, tf.get_cur_state_entry_ccno());
}

void test_qfh_unfaulted() {
    // If the radio is disabled, it doesn't matter how
    // long we've been without comms. There should be no
    // transition to a faulted state.
    {
        // Set initial conditions 
        TestFixture tf{fault_checker_state_t::unfaulted};
        control_cycle_count = PAN::one_day_ccno;
        tf.set_cur_state_entry_ccno(PAN::one_day_ccno);
        
        // Disable radio within the "24 hour" period of this state.
        // Verify that the state machine goes back to "unfaulted".
        tf.disable_radio();
        control_cycle_count = 2 * PAN::one_day_ccno - 1;
        tf.step_and_expect(mission_state_t::manual, fault_checker_state_t::unfaulted);
    }

    // If the radio is enabled, ensure that there's a transition
    // to the forced standby state after 24 hours.
    {
        TestFixture tf{fault_checker_state_t::unfaulted};
        tf.enable_radio();
        control_cycle_count = PAN::one_day_ccno - 1;
        tf.step_and_expect(mission_state_t::manual, fault_checker_state_t::unfaulted);
        tf.step_and_expect(mission_state_t::standby, fault_checker_state_t::forced_standby);
    }
}

void test_qfh_forced_standby() {
    // If it's been more than 24 hours since comms, the fault handler
    // should cause a power cycle and move to the powercycle_1 state.
    {
        TestFixture tf{fault_checker_state_t::forced_standby};
        tf.set_cur_state_entry_ccno(PAN::one_day_ccno);
        control_cycle_count = 2 * PAN::one_day_ccno;
        tf.step_and_expect(mission_state_t::standby, fault_checker_state_t::powercycle_1);
        tf.check_powercycled();
    }
    {
        // Cleanup
        control_cycle_count = 0;
    }

    // If the radio is disabled the state should return to unfaulted immediately.
    {
        TestFixture tf{fault_checker_state_t::forced_standby};
        tf.check_state_returns_to_unfaulted_if_radio_disabled();
    }

    // If we've (recently) received comms within 24 hours since the state transition, the state
    // should return to unfaulted immediately.
    {
        TestFixture tf{fault_checker_state_t::forced_standby};
        tf.check_state_returns_to_unfaulted_if_comms_recvd(PAN::one_day_ccno);
    }
}

void test_qfh_powercycle_1() {
    // If it's been more than 8 hours since comms, the fault handler
    // should cause a power cycle and move to the powercycle_2 state.
    {
        TestFixture tf{fault_checker_state_t::powercycle_1};
        tf.set_cur_state_entry_ccno(PAN::one_day_ccno);
        control_cycle_count = PAN::one_day_ccno + PAN::one_day_ccno / 3;
        tf.step_and_expect(mission_state_t::standby, fault_checker_state_t::powercycle_2);
        tf.check_powercycled();
    }
    {
        // Cleanup
        control_cycle_count = 0;
    }


    // If we've (recently) received comms within 8 hours since the state transition, the state
    // should return to unfaulted immediately.
    {
        TestFixture tf{fault_checker_state_t::powercycle_1};
        tf.check_state_returns_to_unfaulted_if_comms_recvd(PAN::one_day_ccno / 3);
    }
}

void test_qfh_powercycle_2() {
    // If it's been more than 8 hours since comms, the fault handler
    // should cause a power cycle and move to the powercycle_3 state.
    {
        TestFixture tf{fault_checker_state_t::powercycle_2};
        tf.set_cur_state_entry_ccno(PAN::one_day_ccno);
        control_cycle_count = PAN::one_day_ccno + PAN::one_day_ccno / 3;
        tf.step_and_expect(mission_state_t::standby, fault_checker_state_t::powercycle_3);
        tf.check_powercycled();
    }
    {
        // Cleanup
        control_cycle_count = 0;
    }

    // If we've (recently) received comms within 8 hours since the state transition, the state
    // should return to unfaulted immediately.
    {
        TestFixture tf{fault_checker_state_t::powercycle_2};
        tf.check_state_returns_to_unfaulted_if_comms_recvd(PAN::one_day_ccno / 3);
    }
}

void test_qfh_powercycle_3() {
    // If it's been more than 8 hours since comms, the fault handler
    // should move to the safe hold state. There is no fourth power
    // cycle.
    {
        TestFixture tf{fault_checker_state_t::powercycle_3};
        tf.set_cur_state_entry_ccno(PAN::one_day_ccno);
        control_cycle_count = PAN::one_day_ccno + PAN::one_day_ccno / 3;
        tf.step_and_expect(mission_state_t::safehold, fault_checker_state_t::safehold);
        tf.check_not_powercycled();
    }
    {
        // Cleanup
        control_cycle_count = 0;
    }

    // If we've (recently) received comms within 8 hours since the state transition, the state
    // should return to unfaulted immediately.
    {
        TestFixture tf{fault_checker_state_t::powercycle_3};
        tf.check_state_returns_to_unfaulted_if_comms_recvd(PAN::one_day_ccno / 3);
    }
}

void test_qfh_safehold() {
    // If the radio is disabled the state should return to unfaulted immediately.
    {
        TestFixture tf{fault_checker_state_t::safehold};
        tf.check_state_returns_to_unfaulted_if_radio_disabled();
    }

    // If we've (recently) received comms within 24 hours since the state transition, the state
    // should return to unfaulted immediately.
    {
        TestFixture tf{fault_checker_state_t::safehold};
        tf.check_state_returns_to_unfaulted_if_comms_recvd(PAN::one_day_ccno);
    }

    // Otherwise, the machine should suggest safe hold as the recommended mission state.
    {
        TestFixture tf{fault_checker_state_t::safehold};
        tf.set_cur_state_entry_ccno(PAN::one_day_ccno);
        control_cycle_count = PAN::one_day_ccno;

        tf.step_and_expect(mission_state_t::safehold, fault_checker_state_t::safehold);
        control_cycle_count += PAN::one_day_ccno / 2;
        tf.step_and_expect(mission_state_t::safehold, fault_checker_state_t::safehold);
        control_cycle_count += PAN::one_day_ccno / 2 - 1;
        tf.step_and_expect(mission_state_t::safehold, fault_checker_state_t::safehold);
        tf.step_and_expect(mission_state_t::safehold, fault_checker_state_t::safehold);
        tf.step_and_expect(mission_state_t::safehold, fault_checker_state_t::safehold);
        tf.step_and_expect(mission_state_t::safehold, fault_checker_state_t::safehold);
    }
    {
        // Cleanup
        control_cycle_count = 0;
    }
}

int test_mission_manager() {
    UNITY_BEGIN();
    RUN_TEST(test_qfhmock);
    RUN_TEST(test_qfh_initialization);
    RUN_TEST(test_qfh_unfaulted);
    RUN_TEST(test_qfh_forced_standby);
    RUN_TEST(test_qfh_powercycle_1);
    RUN_TEST(test_qfh_powercycle_2);
    RUN_TEST(test_qfh_powercycle_3);
    RUN_TEST(test_qfh_safehold);
    return UNITY_END();
}

#ifdef DESKTOP
int main() {
    return test_mission_manager();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_mission_manager();
}

void loop() {}
#endif
