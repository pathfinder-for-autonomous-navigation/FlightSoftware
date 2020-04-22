#include "test_fault_handlers.hpp"
#include "../FaultHandlerMachineMock.hpp"
#include <fsw/FCCode/MainFaultHandler.hpp>
#include <algorithm>
#include <numeric>
#include "cartesian_product.hpp"

class TestFixtureMainFH {
  protected:
    unsigned int& cc = TimedControlTaskBase::control_cycle_count;
    StateFieldRegistryMock registry;
    std::unique_ptr<MainFaultHandler> fault_handler;

  public:
    // Input state fields to fault handler
    std::shared_ptr<InternalStateField<unsigned char>> radio_state_fp;
    std::shared_ptr<InternalStateField<unsigned int>> radio_last_comms_ccno_fp;
    std::shared_ptr<WritableStateField<bool>> quake_power_cycle_cmd_fp;
    std::shared_ptr<Fault> adcs_wheel1_adc_fault_fp;
    std::shared_ptr<Fault> adcs_wheel2_adc_fault_fp;
    std::shared_ptr<Fault> adcs_wheel3_adc_fault_fp;
    std::shared_ptr<Fault> adcs_wheel_pot_fault_fp;
    std::shared_ptr<Fault> low_batt_fault_fp;
    std::shared_ptr<Fault> prop_failed_pressurize_fault_fp;
    std::shared_ptr<Fault> prop_overpressure_fault_fp;
    std::shared_ptr<ReadableStateField<unsigned char>> piksi_state_fp;
    std::shared_ptr<WritableStateField<unsigned char>> mission_state_fp;
    std::shared_ptr<InternalStateField<unsigned int>> last_rtkfix_ccno_fp;
    std::shared_ptr<InternalStateField<unsigned int>> enter_close_appr_ccno_fp;

    WritableStateField<bool>* fault_handler_enabled_fp = nullptr;
    size_t num_fault_handler_machines = 0;

    TestFixtureMainFH() {
        cc = 0;
        Fault::cc = &cc;

        // Prepare inputs for main fault handler
        radio_state_fp = registry.create_internal_field<unsigned char>("radio.state");
        radio_last_comms_ccno_fp = registry.create_internal_field<unsigned int>("radio.last_comms_ccno");
        quake_power_cycle_cmd_fp = registry.create_writable_field<bool>("gomspace.power_cycle_output1_cmd");

        adcs_wheel1_adc_fault_fp = registry.create_fault("adcs_monitor.wheel1_fault", 1);
        adcs_wheel2_adc_fault_fp = registry.create_fault("adcs_monitor.wheel2_fault", 1);
        adcs_wheel3_adc_fault_fp = registry.create_fault("adcs_monitor.wheel3_fault", 1);
        adcs_wheel_pot_fault_fp = registry.create_fault("adcs_monitor.wheel_pot_fault", 1);
        low_batt_fault_fp = registry.create_fault("gomspace.low_batt", 1);
        prop_failed_pressurize_fault_fp = registry.create_fault("prop.failed_pressurize", 1);
        prop_overpressure_fault_fp = registry.create_fault("prop.overpressured", 1);
        piksi_state_fp = registry.create_readable_field<unsigned char>("piksi.state");
        mission_state_fp = registry.create_writable_field<unsigned char>("pan.state");
        last_rtkfix_ccno_fp = registry.create_internal_field<unsigned int>("piksi.last_rtkfix_ccno");
        enter_close_appr_ccno_fp = registry.create_internal_field<unsigned int>("pan.enter_close_approach_ccno");


        // Construct main fault handler and capture its outputs
        fault_handler = std::make_unique<MainFaultHandler>(registry);
        fault_handler->init();
        fault_handler_enabled_fp = registry.find_writable_field_t<bool>("fault_handler.enabled");
        num_fault_handler_machines = fault_handler->fault_handler_machines.size();

        // Replace all fault handler submachines with mocks
        for(size_t i = 0; i < num_fault_handler_machines; i++) {
            fault_handler->fault_handler_machines[i] = std::make_unique<FaultHandlerMachineMock>(registry);
        }
    }

    /**
     * @brief Set the output of a particular fault machine to a recommended state.
     * 
     * @param idx
     * @param response
     */
    void set_fault_machine_response(size_t idx, fault_response_t response) {
        static_cast<FaultHandlerMachineMock*>(
            fault_handler->fault_handler_machines[idx].get())->set(response);
    }

    /**
     * @brief Enables or disables global fault handling.
     * 
     * @param state If true/false, enable/disable global fault handling.
     */
    void set_fault_handling(bool state) {
        fault_handler_enabled_fp->set(state);
    }

    /**
     * @brief Function to step the main fault handler one control cycle forward.
     * This function also accepts a directive for the outputs of the main
     * fault handler's underlying fault state machines.
     * 
     * @param responses Desired response for the underlying fault machines
     * @return fault_response_t 
     */
    template<size_t N>
    fault_response_t step(const std::array<fault_response_t, N>& responses) 
    {
        assert(responses.size() == num_fault_handler_machines);
        for(size_t i = 0; i < num_fault_handler_machines; i++) {
            set_fault_machine_response(i, responses[i]);
        }

        fault_response_t ret = fault_handler->execute();
        cc++;
        return ret;
    }
};

/**
 * @brief The main fault handler should initially be enabled and
 * produce a ground-controllable flag to enable/disable global fault
 * response.
 */
void test_main_fh_initialization() {
    TestFixtureMainFH tf;
    TEST_ASSERT_NOT_NULL(tf.fault_handler_enabled_fp);
    TEST_ASSERT_TRUE(tf.fault_handler_enabled_fp->get());
}

/**
 * @brief Initially, set there to be no fault response from any sub-machine.
 * Verify that the global fault machine also recommends no response.
 */
void test_main_fh_no_fault() {
    TestFixtureMainFH tf;
    assert(tf.num_fault_handler_machines == 10);

    fault_response_t response = tf.step<10>({
        fault_response_t::none, fault_response_t::none, fault_response_t::none,
        fault_response_t::none, fault_response_t::none, fault_response_t::none,
        fault_response_t::none, fault_response_t::none, fault_response_t::none
    });
    TEST_ASSERT_EQUAL(fault_response_t::none, response);
}

/**
 * @brief Test all combinations of faults that lead to a standby response.
 */
void test_main_fh_standby_fault() {
    TestFixtureMainFH tf;
    assert(tf.num_fault_handler_machines == 10);

    // Produce all combinations of none/standby fault response recommendations.
    static constexpr std::array<fault_response_t, 2> allowed_responses 
        {fault_response_t::none, fault_response_t::standby};
    const std::vector<std::array<fault_response_t, 10>> combos
        = NthCartesianProduct<10>::of(allowed_responses);

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
    TestFixtureMainFH tf;
    assert(tf.num_fault_handler_machines == 10);

    // Produce all combinations of none/standby/safehold fault response recommendations.
    static constexpr std::array<fault_response_t, 3> allowed_responses
        {fault_response_t::none, fault_response_t::standby, fault_response_t::safehold};
    const std::vector<std::array<fault_response_t, 10>> combos 
        = NthCartesianProduct<10>::of(allowed_responses);

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
    TestFixtureMainFH tf;
    assert(tf.num_fault_handler_machines == 10);

    // This is a random combination that definitely causes a fault
    // recommendation to transition to safe hold.
    std::array<fault_response_t, 10> safehold_combo = {
        fault_response_t::safehold, fault_response_t::safehold, 
        fault_response_t::none, fault_response_t::none,
        fault_response_t::none, fault_response_t::none,
        fault_response_t::standby, fault_response_t::safehold,
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
