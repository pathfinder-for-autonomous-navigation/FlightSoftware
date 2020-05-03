//
// Created by athena on 4/21/20.
//

#include "prop_state_t.enum"
#include "PropFaultHandler.h"

#if (defined(UNIT_TEST) && defined(DESKTOP))
#define DD(f_, ...) std::printf((f_), ##__VA_ARGS__)
#else
#define DD(f_, ...) \
    do              \
    {               \
    } while (0)
#endif

PropFaultHandler::PropFaultHandler(StateFieldRegistry &r) : FaultHandlerMachine(r)
{

    // StateFields
    prop_state_fp = find_writable_field<unsigned int>("prop.state", __FILE__, __LINE__);
    max_venting_cycles_fp = find_writable_field<unsigned int>("prop.max_venting_cycles", __FILE__, __LINE__);
    max_pressurizing_cycles = find_writable_field<unsigned int>("prop.max_pressurizing_cycles", __FILE__, __LINE__);
    ctrl_cycles_per_filling_period = find_writable_field<unsigned int>("prop.ctrl_cycles_per_filling", __FILE__, __LINE__);

    ctrl_cycles_per_cooling_period = find_writable_field<unsigned int>("prop.ctrl_cycles_per_cooling", __FILE__, __LINE__);

    // Faults
    overpressure_fault_fp = find_fault("prop.overpressured.base", __FILE__, __LINE__);
    pressurize_fail_fault_fp = find_fault("prop.pressurize_fail.base", __FILE__, __LINE__);
    tank2_temp_high_fault_fp = find_fault("prop.tank2_temp_high.base", __FILE__, __LINE__);
    tank1_temp_high_fault_fp = find_fault("prop.tank1_temp_high.base", __FILE__, __LINE__);
}

fault_response_t PropFaultHandler::execute()
{

    auto prop_state = static_cast<prop_state_t>(prop_state_fp->get());

    // Don't accept schedules if we are handling_fault or venting
    if (prop_state == prop_state_t::handling_fault || prop_state == prop_state_t::venting)
        return fault_response_t::standby;
    else
        return fault_response_t::none;
}

bool PropFaultHandler::both_tanks_want_to_vent() const
{
    bool tank1_wants_to_vent = tank1_temp_high_fault_fp->is_faulted();
    bool tank2_wants_to_vent = (tank2_temp_high_fault_fp->is_faulted() &&
                                overpressure_fault_fp->is_faulted());
    return tank1_wants_to_vent && tank2_wants_to_vent;
}