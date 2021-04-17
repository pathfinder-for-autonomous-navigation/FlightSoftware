#include "QuakeFaultHandler.hpp"
#include "constants.hpp"
#include "radio_state_t.enum"

const unsigned int &cccount = TimedControlTaskBase::control_cycle_count;

QuakeFaultHandler::QuakeFaultHandler(StateFieldRegistry &r) : FaultHandlerMachine(r),
                                                              cur_state("qfh.state", Serializer<unsigned char>(5)),
                                                              qfh_enable_f("qfh.enabled", Serializer<bool>())
{
    radio_state_fp = find_readable_field<unsigned char>("radio.state", __FILE__, __LINE__);
    last_checkin_cycle_fp = find_readable_field<unsigned int>("radio.last_comms_ccno", __FILE__,
                                                              __LINE__);

    radio_power_cycle_fp = find_writable_field<bool>("gomspace.power_cycle_output3_cmd", __FILE__,

                                                     __LINE__);

    cur_state.set(static_cast<unsigned char>(qfh_state_t::unfaulted));
    add_writable_field(cur_state);
    add_writable_field(qfh_enable_f);
    // Default enable to true
    qfh_enable_f.set(true);
}

fault_response_t QuakeFaultHandler::execute()
{
    if (!qfh_enable_f.get()) return fault_response_t::none;

    qfh_state_t state = static_cast<qfh_state_t>(cur_state.get());

    switch (state)
    {
    case qfh_state_t::unfaulted:
        return dispatch_unfaulted();
    case qfh_state_t::forced_standby:
        return dispatch_forced_standby();
    case qfh_state_t::powercycle_1:
        return dispatch_powercycle_1();
    case qfh_state_t::powercycle_2:
        return dispatch_powercycle_2();
    case qfh_state_t::powercycle_3:
        return dispatch_powercycle_3();
    case qfh_state_t::safehold:
        return dispatch_safehold();
    default:
        transition_to(qfh_state_t::unfaulted);
        return fault_response_t::none;
    }
    return fault_response_t::none;
}

void QuakeFaultHandler::transition_to(qfh_state_t next_state)
{
    cur_state.set(static_cast<unsigned char>(next_state));
    cur_state_entry_ccno = cccount;
}

fault_response_t QuakeFaultHandler::dispatch_unfaulted()
{
    if (radio_is_disabled() || less_than_one_day_since_successful_comms())
    {
        return fault_response_t::none;
    }
    else
    {
        transition_to(qfh_state_t::forced_standby);
        return fault_response_t::standby;
    }
}

fault_response_t QuakeFaultHandler::dispatch_forced_standby()
{
    if (in_state_for_more_than_time(PAN::one_day_ccno))
    {
        radio_state_fp->set(static_cast<unsigned int>(radio_state_t::disabled));
        radio_power_cycle_fp->set(true);
        transition_to(qfh_state_t::powercycle_1);
        return fault_response_t::standby;
    }
    else if (radio_is_disabled() || less_than_one_day_since_successful_comms())
    {
        transition_to(qfh_state_t::unfaulted);
        return fault_response_t::none;
    }

    return fault_response_t::standby;
}

fault_response_t QuakeFaultHandler::dispatch_powercycle(qfh_state_t next)
{
    if (cur_state_entry_ccno + 1 >= cccount)
    {
        radio_state_fp->set(static_cast<unsigned int>(radio_state_t::config));
    }
    if (in_state_for_more_than_time(PAN::one_day_ccno / 3))
    {
        if (next != qfh_state_t::safehold)
        {
            radio_state_fp->set(static_cast<unsigned int>(radio_state_t::disabled));
            radio_power_cycle_fp->set(true);
        }
        transition_to(next);
        return fault_response_t::standby;
    }
    else if (less_than_one_day_since_successful_comms())
    {
        transition_to(qfh_state_t::unfaulted);
        return fault_response_t::none;
    }

    return fault_response_t::standby;
}

fault_response_t QuakeFaultHandler::dispatch_powercycle_1()
{
    return dispatch_powercycle(qfh_state_t::powercycle_2);
}

fault_response_t QuakeFaultHandler::dispatch_powercycle_2()
{
    return dispatch_powercycle(qfh_state_t::powercycle_3);
}

fault_response_t QuakeFaultHandler::dispatch_powercycle_3()
{
    return dispatch_powercycle(qfh_state_t::safehold);
}

fault_response_t QuakeFaultHandler::dispatch_safehold()
{
    if (radio_is_disabled() || less_than_one_day_since_successful_comms())
    {
        transition_to(qfh_state_t::unfaulted);
        return fault_response_t::none;
    }
    else
    {
        return fault_response_t::safehold;
    }
}

bool QuakeFaultHandler::less_than_one_day_since_successful_comms() const
{
    return cccount - last_checkin_cycle_fp->get() < PAN::one_day_ccno;
}

bool QuakeFaultHandler::in_state_for_more_than_time(const unsigned int time) const
{
    return cccount - cur_state_entry_ccno >= time;
}

bool QuakeFaultHandler::in_state_for_exact_time(const unsigned int time) const
{
    return cccount - cur_state_entry_ccno == time;
}

bool QuakeFaultHandler::radio_is_disabled() const
{
    return radio_state_fp->get() == static_cast<unsigned char>(radio_state_t::disabled);
}

bool QuakeFaultHandler::radio_is_wait() const
{
    return radio_state_fp->get() == static_cast<unsigned char>(radio_state_t::wait);
}
