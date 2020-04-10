#ifndef TEST_FIXTURE_MAIN_FH_HPP_
#define TEST_FIXTURE_MAIN_FH_HPP_

#include "../StateFieldRegistryMock.hpp"
#include "FaultHandlerMachineMock.hpp"

class TestFixtureMainFH {
  protected:
    unsigned int cc = 0; // Control cycle count
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

    WritableStateField<bool>* fault_handler_enabled_fp = nullptr;
    size_t num_fault_handler_machines = 0;

    /**
     * @brief Construct a new Test Fixture.
     * 
     * @param mock If true, replaces sub-fault handlers with mocks.
     */
    TestFixtureMainFH(bool mock = true);

    /**
     * @brief Set the output of a particular fault machine to a recommended state.
     * 
     * @param idx
     * @param response
     */
    void set_fault_machine_response(size_t idx, fault_response_t response);

    /**
     * @brief Enables or disables global fault handling.
     * 
     * @param state If true/false, enable/disable global fault handling.
     */
    void set_fault_handling(bool state);

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

#endif
