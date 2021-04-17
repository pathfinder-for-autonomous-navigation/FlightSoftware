#include "test_fixture_main_fh.hpp"
#include "FaultHandlerMachineMock.hpp"

TestFixtureMainFHBase::TestFixtureMainFHBase()
{
    cc = 0;

    // Prepare inputs for main fault handler
    mission_state_fp = registry.create_writable_field<unsigned char>("pan.state");
    radio_state_fp = registry.create_readable_field<unsigned char>("radio.state");
    radio_last_comms_ccno_fp = registry.create_readable_field<unsigned int>("radio.last_comms_ccno");

    piksi_state_fp = registry.create_readable_field<unsigned char>("piksi.state");
    enter_close_approach_ccno_fp = registry.create_internal_field<unsigned int>("pan.enter_close_approach_ccno");

    quake_power_cycle_cmd_fp = registry.create_writable_field<bool>("gomspace.power_cycle_output3_cmd");
    adcs_wheel1_adc_fault_fp = registry.create_fault("adcs_monitor.wheel1_fault", 1);
    adcs_wheel2_adc_fault_fp = registry.create_fault("adcs_monitor.wheel2_fault", 1);
    adcs_wheel3_adc_fault_fp = registry.create_fault("adcs_monitor.wheel3_fault", 1);
    adcs_wheel_pot_fault_fp = registry.create_fault("adcs_monitor.wheel_pot_fault", 1);
    low_batt_fault_fp = registry.create_fault("gomspace.low_batt", 1);
    prop_failed_pressurize_fault_fp = registry.create_fault("prop.pressurize_fail", 1);
    prop_overpressure_fault_fp = registry.create_fault("prop.overpressured", 1);

    // Construct main fault handler and capture its outputs
    fault_handler = std::make_unique<MainFaultHandler>(registry);
    fault_handler->init();
    fault_handler_enabled_fp = registry.find_writable_field_t<bool>("fault_handler.enabled");
}

void TestFixtureMainFHBase::set_fault_handling(bool state)
{
    fault_handler_enabled_fp->set(state);
}

TestFixtureMainFHMocked::TestFixtureMainFHMocked() : TestFixtureMainFHBase()
{
    num_fault_handler_machines = fault_handler->fault_handler_machines.size();

    // Replace all fault handler submachines with mocks
    for (size_t i = 0; i < num_fault_handler_machines; i++)
    {
        fault_handler->fault_handler_machines[i] = std::make_unique<FaultHandlerMachineMock>(registry);
    }
}

void TestFixtureMainFHMocked::set_fault_machine_response(size_t idx, fault_response_t response)
{
    static_cast<FaultHandlerMachineMock *>(
        fault_handler->fault_handler_machines[idx].get())
        ->set(response);
}

TestFixtureMainFHEndToEnd::TestFixtureMainFHEndToEnd()
{
    // Set the mission state to "standby" so that all fault handlers are activated.
    set(mission_state_t::standby);
}

fault_response_t TestFixtureMainFHEndToEnd::step()
{
    fault_response_t ret = fault_handler->execute();
    cc++;
    return ret;
}

void TestFixtureMainFHEndToEnd::set(mission_state_t state)
{
    mission_state_fp->set(static_cast<unsigned char>(state));
}

void TestFixtureMainFHEndToEnd::set(radio_state_t state)
{
    radio_state_fp->set(static_cast<unsigned char>(state));
}

void TestFixtureMainFHEndToEnd::set_last_comms_ccno(unsigned int ccno)
{
    radio_last_comms_ccno_fp->set(ccno);
}
