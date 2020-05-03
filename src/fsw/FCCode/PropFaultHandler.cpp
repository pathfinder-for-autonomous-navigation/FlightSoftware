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

    // Save these values at startup
    saved_max_pressurizing_cycles = max_pressurizing_cycles->get();
    saved_cooling_cycles = ctrl_cycles_per_cooling_period->get();
}

fault_response_t PropFaultHandler::execute()
{

    auto prop_state = static_cast<prop_state_t>(prop_state_fp->get());

    if (prop_state != prop_state_t::handling_fault && prop_state != prop_state_t::venting)
        return fault_response_t::none;

    if (both_tanks_want_to_vent())
        handle_both_tanks_want_to_vent();
    else
        handle_restore_values();

    // Don't accept schedules if we are handling_fault or venting
    return fault_response_t::standby;
}

bool PropFaultHandler::both_tanks_want_to_vent() const
{
    bool tank1_wants_to_vent = tank1_temp_high_fault_fp->is_faulted();
    bool tank2_wants_to_vent = (tank2_temp_high_fault_fp->is_faulted() ||
                                overpressure_fault_fp->is_faulted());
    return tank1_wants_to_vent && tank2_wants_to_vent;
}

void PropFaultHandler::handle_both_tanks_want_to_vent()
{
    DD("Both tanks want to vent\n");
    // If this is the first cycle at which we are venting, then adjust the values
    if (num_cycles_both_venting == 0)
    {
        DD("Manipulating Prop Controller values\n");
        // Saved the max_pressue cycles and ctrl_cycles_per_cooling value
        saved_max_pressurizing_cycles = max_pressurizing_cycles->get();
        saved_cooling_cycles = ctrl_cycles_per_cooling_period->get();
        // Set max pressurizing cycle to 1
        max_pressurizing_cycles->set(1);
        // Set ctrl_cycles_per_cooling to 1s
        ctrl_cycles_per_cooling_period->set(ctrl_cycles_per_filling_period->get());

        ++num_cycles_both_venting;
    }
    else
    {
        // Only increment when we are in handling_fault because that is when we
        // are switching
        if (prop_state_fp->get() == static_cast<unsigned char>(prop_state_t::handling_fault))
        {
            DD("Incrementing ++num_cycles_both_venting\n");
            ++num_cycles_both_venting;
        }
    }

    // If we have exceeded max_venting_cycles, then set us to disabled
    if (num_cycles_both_venting > max_venting_cycles_fp->get())
    {
        DD("Num_cycles_both_venting exceeded max_venting_cycles --> Going into disabled\n");
        prop_state_fp->set(static_cast<unsigned char>(prop_state_t::disabled));
    }
}

void PropFaultHandler::handle_restore_values()
{
    DD("Restoring values\n");
    max_pressurizing_cycles->set(saved_max_pressurizing_cycles);
    ctrl_cycles_per_cooling_period->set(saved_cooling_cycles);
    num_cycles_both_venting = 0;
}