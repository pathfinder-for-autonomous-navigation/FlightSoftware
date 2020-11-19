//
// Created by athena on 4/21/20.
//

#include "prop_state_t.enum"
#include "PropController.hpp"
#include "PropFaultHandler.h"

#if (defined(UNIT_TEST) && defined(DESKTOP))
#define DD(f_, ...) std::printf((f_), ##__VA_ARGS__)
#else
#define DD(f_, ...) \
    do              \
    {               \
    } while (0)
#endif

PropFaultHandler::PropFaultHandler(StateFieldRegistry &r)
    : FaultHandlerMachine(r)
{
}

void PropFaultHandler::init()
{
    // StateFields
    prop_state_fp = find_writable_field<unsigned int>("prop.state", __FILE__, __LINE__);
    max_venting_cycles_fp = find_writable_field<unsigned int>("prop.max_venting_cycles", __FILE__, __LINE__);

    // Faults
    overpressure_fault_fp = find_fault("prop.overpressured.base", __FILE__, __LINE__);
    tank2_temp_high_fault_fp = find_fault("prop.tank2_temp_high.base", __FILE__, __LINE__);
    tank1_temp_high_fault_fp = find_fault("prop.tank1_temp_high.base", __FILE__, __LINE__);

    // Save these values at startup
    saved_max_venting_cycles = max_venting_cycles_fp->get();
}

fault_response_t PropFaultHandler::execute()
{
    if (PropState::controller == nullptr)
        return fault_response_t::none;

    if (has_not_init)
    {
        init();
        has_not_init = false;
    }

    auto prop_state = static_cast<prop_state_t>(prop_state_fp->get());
    // Only care about prop when it is venting or in handling_fault
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
    // If this is the first cycle at which we are venting, then adjust the values
    if (num_cycles_both_venting == 0)
    {
        DD("Manipulating Prop Controller values\n");
        // Saved the max venting cycles
        saved_max_venting_cycles = max_venting_cycles_fp->get();
        // Set max venting cycle to 1
        max_venting_cycles_fp->set(1);

        ++num_cycles_both_venting;
    }
    else if (prop_state_fp->get() == static_cast<unsigned char>(prop_state_t::handling_fault))
    {
        // Only increment when we are in handling_fault because that is when we
        // are switching
        ++num_cycles_both_venting;
        DD("Incrementing ++num_cycles_both_venting %zu\n", num_cycles_both_venting);

        // If we have exceeded the original max_venting_cycles (20)
        if (num_cycles_both_venting > saved_max_venting_cycles)
        {
            DD("Num_cycles_both_venting exceeded max_venting_cycles --> Going into disabled\n");
            prop_state_fp->set(static_cast<unsigned char>(prop_state_t::disabled));
            // Manually set this back
            handle_restore_values();
        }
    }
}

void PropFaultHandler::handle_restore_values()
{
    DD("Restoring max_venting_cycles to %zu\n", saved_max_venting_cycles);
    max_venting_cycles_fp->set(saved_max_venting_cycles);
    num_cycles_both_venting = 0;
}