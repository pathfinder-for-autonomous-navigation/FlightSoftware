#include "MainFaultHandler.hpp"
#include "SimpleFaultHandler.hpp"
#include "PropOverpressureFaultHandler.hpp"
#include "QuakeFaultHandler.hpp"
#include "PiksiFaultHandler.hpp"

MainFaultHandler::MainFaultHandler(StateFieldRegistry& r) :
        FaultHandlerMachine(r),
        fault_handler_enabled_f("fault_handler.enabled", Serializer<bool>())
{
    add_writable_field(fault_handler_enabled_f);
    fault_handler_enabled_f.set(true);
}

void MainFaultHandler::init() {
    // Populate inputs (and retrieve pointer for some outputs)
    std::vector<Fault*> active_list_0_safehold_super_simple_faults {
        find_fault("gomspace.low_batt", __FILE__, __LINE__),
    };

    std::vector<Fault*> active_list_1_safehold_super_simple_faults {
        find_fault("adcs_monitor.wheel1_fault", __FILE__, __LINE__),
        find_fault("adcs_monitor.wheel2_fault", __FILE__, __LINE__),
        find_fault("adcs_monitor.wheel3_fault", __FILE__, __LINE__),
        find_fault("adcs_monitor.wheel_pot_fault", __FILE__, __LINE__),
        find_fault("prop.overpressured", __FILE__, __LINE__)
    };

    std::vector<Fault*> active_list_1_standby_super_simple_faults {
        find_fault("prop.failed_pressurize", __FILE__, __LINE__),
    };

    for(Fault* fault : active_list_0_safehold_super_simple_faults) {
        fault_handler_machines.push_back(
            std::make_unique<SuperSimpleFaultHandler>(_registry, fault,
                SimpleFaultHandler::active_state_lists[0], mission_state_t::safehold)
        );
    }

    for(Fault* fault : active_list_1_safehold_super_simple_faults) {
        fault_handler_machines.push_back(
            std::make_unique<SuperSimpleFaultHandler>(_registry, fault,
                SimpleFaultHandler::active_state_lists[1], mission_state_t::safehold)
        );
    }

    for(Fault* fault : active_list_1_standby_super_simple_faults) {
        fault_handler_machines.push_back(
            std::make_unique<SuperSimpleFaultHandler>(_registry, fault,
                SimpleFaultHandler::active_state_lists[1], mission_state_t::standby)
        );
    }

    fault_handler_machines.push_back(std::make_unique<PropOverpressureFaultHandler>(_registry));

    fault_handler_machines.push_back(std::make_unique<QuakeFaultHandler>(_registry));
    fault_handler_machines.push_back(std::make_unique<PiksiFaultHandler>(_registry));
}

fault_response_t MainFaultHandler::execute() {
    // By default, or if the fault handling is globally disabled, recommend no action.
    fault_response_t ret = fault_response_t::none;
    if (!fault_handler_enabled_f.get()) return ret;

    for(std::unique_ptr<FaultHandlerMachine>& m : fault_handler_machines) {
        const fault_response_t response = m->execute();

        if (response == fault_response_t::none) continue;
        else if (response == fault_response_t::safehold) {
            return fault_response_t::safehold;
        }
        else {
            ret = fault_response_t::standby;
        }
    }

    return ret;
}
