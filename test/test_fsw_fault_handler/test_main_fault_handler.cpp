#include "test_fault_handlers.hpp"
#include "test_fixture_main_fh.hpp"
#include <algorithm>
#include <numeric>
#include "cartesian_product.hpp"

static constexpr unsigned int num_fault_machines = 10;

/**
 * @brief The main fault handler should initially be enabled and
 * produce a ground-controllable flag to enable/disable global fault
 * response.
 */
void test_main_fh_initialization() {
    TestFixtureMainFHMocked tf;
    TEST_ASSERT_NOT_NULL(tf.fault_handler_enabled_fp);
    TEST_ASSERT_TRUE(tf.fault_handler_enabled_fp->get());
}

/**
 * @brief Initially, set there to be no fault response from any sub-machine.
 * Verify that the global fault machine also recommends no response.
 */
void test_main_fh_no_fault() {
    TestFixtureMainFHMocked tf;
    assert(tf.num_fault_handler_machines == num_fault_machines);

    std::array<fault_response_t, num_fault_machines> null_fault_responses = 
    {
        fault_response_t::none, fault_response_t::none, fault_response_t::none,
        fault_response_t::none, fault_response_t::none, fault_response_t::none,
        fault_response_t::none, fault_response_t::none, fault_response_t::none
    };

    fault_response_t response = tf.step<num_fault_machines>(null_fault_responses);
    TEST_ASSERT_EQUAL(fault_response_t::none, response);
}

/**
 * @brief Test all combinations of faults that lead to a standby response.
 */
void test_main_fh_standby_fault() {
    TestFixtureMainFHMocked tf;
    assert(tf.num_fault_handler_machines == num_fault_machines);

    // Produce all combinations of none/standby fault response recommendations.
    static constexpr std::array<fault_response_t, 2> allowed_responses 
        {fault_response_t::none, fault_response_t::standby};
    const std::vector<std::array<fault_response_t, num_fault_machines>> combos
        = NthCartesianProduct<num_fault_machines>::of(allowed_responses);

    for(auto const & combo : combos) {
        // Verify that there is at least one fault machine
        // that will produce a recommendation for standby. If there is
        // not, skip this combo.
        const bool combo_valid = std::accumulate(
            combo.begin(), combo.end(), false, 
            [](bool valid, fault_response_t response) {
                return valid || response == fault_response_t::standby;
            });
        if (!combo_valid) continue;

        fault_response_t response = tf.step(combo);
        TEST_ASSERT_EQUAL(fault_response_t::standby, response);
    }
}

// Test all combinations of faults that lead to a safehold response.
void test_main_fh_safehold_fault() {
    TestFixtureMainFHMocked tf;
    assert(tf.num_fault_handler_machines == num_fault_machines);

    // Produce all combinations of none/standby/safehold fault response recommendations.
    static constexpr std::array<fault_response_t, 3> allowed_responses
        {fault_response_t::none, fault_response_t::standby, fault_response_t::safehold};
    const std::vector<std::array<fault_response_t, num_fault_machines>> combos 
        = NthCartesianProduct<num_fault_machines>::of(allowed_responses);

    for(auto const & combo : combos) {
        // Verify that there is at least one fault machine in this combo
        // that will produce a recommendation for safehold. If there is not,
        // skip this combo.
        const bool combo_valid = std::accumulate(
            combo.begin(), combo.end(), false, 
            [](bool valid, fault_response_t response) {
                return valid || response == fault_response_t::safehold;
            });
        if (!combo_valid) continue;

        fault_response_t response = tf.step(combo);
        TEST_ASSERT_EQUAL(fault_response_t::safehold, response);
    }
}

/**
 * @brief Check that the state field that can enable or disable fault
 * responses works.
 */
void test_main_fh_toggle_handling() {
    TestFixtureMainFHMocked tf;
    assert(tf.num_fault_handler_machines == num_fault_machines);

    // This is a random combination that definitely causes a fault
    // recommendation to transition to safe hold.
    std::array<fault_response_t, num_fault_machines> safehold_combo = {
        fault_response_t::safehold, fault_response_t::safehold, 
        fault_response_t::none,     fault_response_t::none,
        fault_response_t::none,     fault_response_t::none,
        fault_response_t::standby,  fault_response_t::safehold,
        fault_response_t::none
    };

    // If some fault machines recommend safehold, the main fault handler
    // definitely recommends safehold.
    fault_response_t response = tf.step(safehold_combo);
    TEST_ASSERT_EQUAL(fault_response_t::safehold, response);

    // Now, we disable global fault handling, and observe that the
    // main fault handler recommends no mission state.
    tf.set_fault_handling(false);
    response = tf.step(safehold_combo);
    TEST_ASSERT_EQUAL(fault_response_t::none, response);

    // If we re-enable global fault handling, we start receiving the
    // recommendation of safehold again.
    tf.set_fault_handling(true);
    response = tf.step(safehold_combo);
    TEST_ASSERT_EQUAL(fault_response_t::safehold, response);
}

void test_main_fault_handler() {
    RUN_TEST(test_main_fh_initialization);
    RUN_TEST(test_main_fh_no_fault);
    RUN_TEST(test_main_fh_standby_fault);
    RUN_TEST(test_main_fh_safehold_fault);
    RUN_TEST(test_main_fh_toggle_handling);
}
