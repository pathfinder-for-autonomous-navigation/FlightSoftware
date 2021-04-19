#include "MainFaultHandler.hpp"
#include "SimpleFaultHandler.hpp"
#include "PropFaultHandler.h"
#include "QuakeFaultHandler.hpp"
#include "PiksiFaultHandler.hpp"

MainFaultHandler::MainFaultHandler(StateFieldRegistry &r)
    : FaultHandlerMachine(r),
      fault_handler_enabled_f("fault_handler.enabled", Serializer<bool>())
{
    add_writable_field(fault_handler_enabled_f);
    fault_handler_enabled_f.set(true);
}

void MainFaultHandler::init()
{
    // Populate inputs (and retrieve pointer for some outputs)
    std::array<Fault *, 1> const active_list_0_safehold_super_simple_faults{
        FIND_FAULT(gomspace.low_batt.base)
    };

    std::array<Fault *, 4> const active_list_1_safehold_super_simple_faults{
        FIND_FAULT(adcs_monitor.wheel1_fault.base),
        FIND_FAULT(adcs_monitor.wheel2_fault.base),
        FIND_FAULT(adcs_monitor.wheel3_fault.base),
        FIND_FAULT(adcs_monitor.wheel_pot_fault.base)
    };

    std::array<Fault *, 1> const active_list_2_standby_super_simple_faults {
        FIND_FAULT(prop.pressurize_fail.base)
    };

    std::array<Fault*, 1> const active_list_3_safehold_super_simple_faults {
        FIND_FAULT(attitude_estimator.fault.base)
    };

    for (auto *fault : active_list_0_safehold_super_simple_faults)
    {
        fault_handler_machines.push_back(
            std::make_unique<SuperSimpleFaultHandler>(_registry, fault,
                SimpleFaultHandler::active_state_lists[0], mission_state_t::safehold)
        );
    }

    for (auto *fault : active_list_1_safehold_super_simple_faults)
    {
        fault_handler_machines.push_back(
            std::make_unique<SuperSimpleFaultHandler>(_registry, fault,
                SimpleFaultHandler::active_state_lists[1], mission_state_t::safehold)
        );
    }

    for(auto *fault : active_list_2_standby_super_simple_faults) {
        fault_handler_machines.push_back(
            std::make_unique<SuperSimpleFaultHandler>(_registry, fault,
                SimpleFaultHandler::active_state_lists[2], mission_state_t::standby)
        );
    }

    for (auto *fault : active_list_3_safehold_super_simple_faults) {
        fault_handler_machines.push_back(
            std::make_unique<SuperSimpleFaultHandler>(_registry, fault,
                SimpleFaultHandler::active_state_lists[3], mission_state_t::safehold)
        );
    }

    fault_handler_machines.push_back(std::make_unique<QuakeFaultHandler>(_registry));
    fault_handler_machines.push_back(std::make_unique<PiksiFaultHandler>(_registry));
    fault_handler_machines.push_back(std::make_unique<PropFaultHandler>(_registry));
}

fault_response_t MainFaultHandler::execute()
{
    // By default, or if the fault handling is globally disabled, recommend no action.
    fault_response_t ret = fault_response_t::none;
    if (!fault_handler_enabled_f.get())
        return ret;

    for (std::unique_ptr<FaultHandlerMachine> &m : fault_handler_machines)
    {
        const fault_response_t response = m->execute();

        if (response == fault_response_t::none)
            continue;
        else if (response == fault_response_t::safehold)
        {
            return fault_response_t::safehold;
        }
        else
        {
            ret = fault_response_t::standby;
        }
    }

    return ret;
}
