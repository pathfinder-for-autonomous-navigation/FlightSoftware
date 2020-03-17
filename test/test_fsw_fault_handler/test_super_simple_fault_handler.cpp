#include "test_fault_handlers.hpp"
#include <fsw/FCCode/SimpleFaultHandler.hpp>

class TestFixtureSuperSimpleFH {
  protected:
    unsigned int cc = 0; // Control cycle count
    StateFieldRegistryMock registry;

  public:
    std::shared_ptr<WritableStateField<unsigned char>> mission_state_fp;
    std::shared_ptr<Fault> fault_fp;
    std::unique_ptr<SuperSimpleFaultHandler> fault_handler;
    const std::vector<mission_state_t> active_states {mission_state_t::follower};

    TestFixtureSuperSimpleFH(mission_state_t recommended_state) : registry() {
        mission_state_fp = registry.create_writable_field<unsigned char>("pan.state", 12);
        SimpleFaultHandler::set_mission_state_ptr(mission_state_fp.get());

        fault_fp = registry.create_fault("fault", 1, cc);

        fault_handler = std::make_unique<SuperSimpleFaultHandler>(
            registry, fault_fp.get(), active_states, recommended_state);

        set(mission_state_t::follower); // Start in an active state for the fault handler.
    }

    void set(mission_state_t state) {
        mission_state_fp->set(static_cast<unsigned char>(state));
    }

    fault_response_t step(bool signal_fault) {
        if (signal_fault) fault_fp->signal();
        else fault_fp->unsignal();

        fault_response_t ret = fault_handler->execute();
        cc++;
        return ret;
    }
};

/**
 * @brief Test a simple fault handler that recommends the safehold
 * state when its underlying fault is faulted.
 */
void test_super_simple_fh_safehold() {
    TestFixtureSuperSimpleFH tf(mission_state_t::safehold);

    fault_response_t response = tf.step(false);
    
    // When fault is unsignaled, the recommended fault response should be ignorable.
    TEST_ASSERT_EQUAL(fault_response_t::none, response);

    // Signal the fault. Now, the recommended fault response should be to go to safehold.
    tf.step(true); response = tf.step(true);
    TEST_ASSERT_EQUAL(fault_response_t::safehold, response);
    
    // Unsignal the fault. The fault response should be ignorable again.
    response = tf.step(false);
    TEST_ASSERT_EQUAL(fault_response_t::none, response);
}

/**
 * @brief Test a simple fault handler that recommends the standby
 * state when its underlying fault is faulted.
 */
void test_super_simple_fh_standby() {
    TestFixtureSuperSimpleFH tf(mission_state_t::standby);

    fault_response_t response = tf.step(false);
    
    // When fault is unsignaled, the recommended fault response should be ignorable.
    TEST_ASSERT_EQUAL(fault_response_t::none, response);

    // Signal the fault. Now, the recommended fault response should be to go to standby.
    tf.step(true); response = tf.step(true);
    TEST_ASSERT_EQUAL(fault_response_t::standby, response);

    // Unsignal the fault. The fault response should be ignorable again.
    response = tf.step(false);
    TEST_ASSERT_EQUAL(fault_response_t::none, response);
}

/**
 * @brief Test that a simple fault handler does not recommend any
 * fault response when it is in a non-active mission state.
 */
void test_super_simple_fh_active_states() {
    // The test begins in an active state for the fault handler.
    TestFixtureSuperSimpleFH tf(mission_state_t::standby);

    // Get the fault to be signaled. The recommended fault response should be to go to standby.
    tf.step(true); fault_response_t response = tf.step(true);
    TEST_ASSERT_EQUAL(fault_response_t::standby, response);

    // Move the mission state into a non-active state. The recommended fault response should
    // become "no response" even though the fault has remained signaled.
    tf.set(mission_state_t::startup);
    response = tf.step(true);
    TEST_ASSERT_EQUAL(fault_response_t::none, response);

    // If we enter a non-active state with the fault being unsignaled, and then signal the
    // fault, the fault handler still suggests no response.
    tf.step(false); tf.step(true); response = tf.step(true);
    TEST_ASSERT_EQUAL(fault_response_t::none, response);

    // Going back into an active state with the fault triggered causes the fault handler
    // to suggest the "go to standby" response. 
    tf.set(mission_state_t::follower);
    response = tf.step(true);
    TEST_ASSERT_EQUAL(fault_response_t::standby, response);

    // Suppose we start in a non-active state, and the fault is just about to be signaled.
    // If we transition to an active state, the fault will be triggered.
    tf.set(mission_state_t::startup);
    tf.step(false);
    tf.step(true);
    tf.set(mission_state_t::follower);
    response = tf.step(true);
    TEST_ASSERT_EQUAL(fault_response_t::standby, response);
}

void test_super_simple_fault_handlers() {
    RUN_TEST(test_super_simple_fh_safehold);
    RUN_TEST(test_super_simple_fh_standby);
    RUN_TEST(test_super_simple_fh_active_states);
}
