#ifndef TEST_FIXTURE_MAIN_FH_HPP_
#define TEST_FIXTURE_MAIN_FH_HPP_

#include "../StateFieldRegistryMock.hpp"
#include <fsw/FCCode/fault_response_t.enum>
#include <fsw/FCCode/MainFaultHandler.hpp>
#include <fsw/FCCode/radio_state_t.enum>

/**
 * @brief This class is the base class for test fixtures
 * for testing the MainFaultHandler. It initializes the inputs 
 * required by the Handler and stores pointers to its outputs.
 */
class TestFixtureMainFHBase
{
protected:
    StateFieldRegistryMock registry;
    std::unique_ptr<MainFaultHandler> fault_handler;

public:
    unsigned int &cc = TimedControlTaskBase::control_cycle_count; // Control cycle count

    // Input state fields to fault handler
    std::shared_ptr<WritableStateField<unsigned char>> mission_state_fp;
    std::shared_ptr<ReadableStateField<unsigned char>> radio_state_fp;
    std::shared_ptr<ReadableStateField<unsigned int>> radio_last_comms_ccno_fp;
    std::shared_ptr<WritableStateField<bool>> quake_power_cycle_cmd_fp;

    std::shared_ptr<ReadableStateField<unsigned char>> piksi_state_fp;
    std::shared_ptr<InternalStateField<unsigned int>> piksi_lastfix_ccno_fp;
    std::shared_ptr<InternalStateField<unsigned int>> enter_close_approach_ccno_fp;

    std::shared_ptr<Fault> adcs_wheel1_adc_fault_fp;
    std::shared_ptr<Fault> adcs_wheel2_adc_fault_fp;
    std::shared_ptr<Fault> adcs_wheel3_adc_fault_fp;
    std::shared_ptr<Fault> adcs_wheel_pot_fault_fp;
    std::shared_ptr<Fault> low_batt_fault_fp;
    std::shared_ptr<Fault> prop_failed_pressurize_fault_fp;
    std::shared_ptr<Fault> prop_overpressure_fault_fp;
    std::shared_ptr<Fault> attitude_estimator_fault_fp;

    WritableStateField<bool> *fault_handler_enabled_fp = nullptr;

    /**
     * @brief Construct a new Test Fixture.
     */
    TestFixtureMainFHBase();

    /**
     * @brief Enables or disables global fault handling.
     * 
     * @param state If true/false, enable/disable global fault handling.
     */
    void set_fault_handling(bool state);
};

/**
 * @brief This test fixture replaces the submachines of the main fault
 * handler with mocked fault handlers so that it's easy to control
 * the submachine responses. This allows exhaustive testing of all fault
 * combinations underneath the MainFaultHandler.
 */
class TestFixtureMainFHMocked : public TestFixtureMainFHBase
{
public:
    size_t num_fault_handler_machines = 0;

    /**
     * @brief Construct a new Test Fixture.
     */
    TestFixtureMainFHMocked();

    /**
     * @brief Set the output of a particular fault machine to a recommended state.
     * 
     * @param idx
     * @param response
     */
    void set_fault_machine_response(size_t idx, fault_response_t response);

    /**
     * @brief Function to step the main fault handler one control cycle forward.
     * This function also accepts a directive for the outputs of the main
     * fault handler's underlying fault state machines.
     * 
     * @param responses Desired response for the underlying fault machines
     * @return fault_response_t
     */
    template <size_t N>
    fault_response_t step(const std::array<fault_response_t, N> &responses)
    {
        assert(responses.size() == num_fault_handler_machines);
        for (size_t i = 0; i < num_fault_handler_machines; i++)
        {
            set_fault_machine_response(i, responses[i]);
        }

        fault_response_t ret = fault_handler->execute();
        cc++;
        return ret;
    }
};

/**
 * @brief This test fixture does not modify the submachine handlers
 * of the MainFaultHandler in the way that TestFixtureMainFHMocked does.
 * Therefore this fixture can be used in end-to-end testing of the
 * MainFaultHandler.
 */
class TestFixtureMainFHEndToEnd : public TestFixtureMainFHBase
{
public:
    TestFixtureMainFHEndToEnd();
    fault_response_t step();
    void set(mission_state_t state);
    void set(radio_state_t state);
    void set_last_comms_ccno(unsigned int ccno);
};

#endif
