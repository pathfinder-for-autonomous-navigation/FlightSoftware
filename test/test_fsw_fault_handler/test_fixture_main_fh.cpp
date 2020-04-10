#include "test_fixture_main_fh.hpp"

TestFixtureMainFH::TestFixtureMainFH(bool mock) {
    // Prepare inputs for main fault handler
    radio_state_fp = registry.create_internal_field<unsigned char>("radio.state");
    radio_last_comms_ccno_fp = registry.create_internal_field<unsigned int>("radio.last_comms_ccno");
    quake_power_cycle_cmd_fp = registry.create_writable_field<bool>("gomspace.power_cycle_output1_cmd");
    adcs_wheel1_adc_fault_fp = registry.create_fault("adcs_monitor.wheel1_fault", 1, cc);
    adcs_wheel2_adc_fault_fp = registry.create_fault("adcs_monitor.wheel2_fault", 1, cc);
    adcs_wheel3_adc_fault_fp = registry.create_fault("adcs_monitor.wheel3_fault", 1, cc);
    adcs_wheel_pot_fault_fp = registry.create_fault("adcs_monitor.wheel_pot_fault", 1, cc);
    low_batt_fault_fp = registry.create_fault("gomspace.low_batt", 1, cc);
    prop_failed_pressurize_fault_fp = registry.create_fault("prop.failed_pressurize", 1, cc);
    prop_overpressure_fault_fp = registry.create_fault("prop.overpressured", 1, cc);

    // Construct main fault handler and capture its outputs
    fault_handler = std::make_unique<MainFaultHandler>(registry);
    fault_handler->init();
    fault_handler_enabled_fp = registry.find_writable_field_t<bool>("fault_handler.enabled");
    num_fault_handler_machines = fault_handler->fault_handler_machines.size();

    if (mock) {
        // Replace all fault handler submachines with mocks
        for(size_t i = 0; i < num_fault_handler_machines; i++) {
            fault_handler->fault_handler_machines[i] = std::make_unique<FaultHandlerMachineMock>(registry);
        }
    }
}

void TestFixtureMainFH::set_fault_machine_response(size_t idx, fault_response_t response) {
    static_cast<FaultHandlerMachineMock*>(
        fault_handler->fault_handler_machines[idx].get())->set(response);
}

void TestFixtureMainFH::set_fault_handling(bool state) {
    fault_handler_enabled_fp->set(state);
}
