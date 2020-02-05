#include "QuakeFaultChecker.hpp"
#include "constants.hpp"
#include "radio_state_t.enum"

QuakeFaultChecker::QuakeFaultChecker(StateFieldRegistry& r) : ControlTask<mission_state_t>(r) {
    radio_state_fp        = find_internal_field<unsigned char>("radio.state", __FILE__, __LINE__);
    last_checkin_cycle_fp = find_internal_field<unsigned int>("radio.last_comms_ccno", __FILE__,
                                                                  __LINE__);
    power_cycle_radio_fp  = find_writable_field<bool>("gomspace.power_cycle_output1_cmd", __FILE__,
                                                          __LINE__);
}

mission_state_t QuakeFaultChecker::execute() {
    switch(cur_state) {
        case fault_checker_state_t::unfaulted:      return dispatch_unfaulted(); break;
        case fault_checker_state_t::forced_standby: return dispatch_forced_standby(); break;
        case fault_checker_state_t::powercycle_1:   return dispatch_powercycle_1(); break;
        case fault_checker_state_t::powercycle_2:   return dispatch_powercycle_2(); break;
        case fault_checker_state_t::powercycle_3:   return dispatch_powercycle_3(); break;
        case fault_checker_state_t::safehold:       return dispatch_safehold(); break;
    }
}

void QuakeFaultChecker::transition_to(fault_checker_state_t next_state) {
    cur_state = next_state;
    cur_state_entry_ccno = control_cycle_count;
}

mission_state_t QuakeFaultChecker::dispatch_unfaulted() {
    const bool is_radio_disabled =
        radio_state_fp->get() == static_cast<unsigned char>(radio_state_t::disabled);
    const bool is_comms_blacked_out =
        (control_cycle_count - last_checkin_cycle_fp->get()) >= PAN::one_day_ccno;

    if (!is_radio_disabled && is_comms_blacked_out) {
        transition_to(fault_checker_state_t::forced_standby);
        return mission_state_t::standby;
    }
    else {
        return mission_state_t::manual;
    }
}

mission_state_t QuakeFaultChecker::dispatch_forced_standby() {
    if (control_cycle_count - cur_state_entry_ccno >= PAN::one_day_ccno) {
        power_cycle_radio_fp->set(true);
        transition_to(fault_checker_state_t::powercycle_1);
        return mission_state_t::standby;
    }
    else {
        return mission_state_t::manual;
    }
}

mission_state_t QuakeFaultChecker::dispatch_powercycle_1() {
    if (control_cycle_count - cur_state_entry_ccno >= PAN::one_day_ccno / 3) {
        power_cycle_radio_fp->set(true);
        transition_to(fault_checker_state_t::powercycle_2);
        return mission_state_t::standby;
    }
    else {
        return mission_state_t::manual;
    }
}

mission_state_t QuakeFaultChecker::dispatch_powercycle_2() {
    if (control_cycle_count - cur_state_entry_ccno >= PAN::one_day_ccno / 3) {
        power_cycle_radio_fp->set(true);
        transition_to(fault_checker_state_t::powercycle_3);
        return mission_state_t::standby;
    }
    else {
        return mission_state_t::manual;
    }
}

mission_state_t QuakeFaultChecker::dispatch_powercycle_3() {
    if (control_cycle_count - cur_state_entry_ccno >= PAN::one_day_ccno / 3) {
        transition_to(fault_checker_state_t::safehold);
        return mission_state_t::safehold;
    }
    else {
        return mission_state_t::manual;
    }
}

mission_state_t QuakeFaultChecker::dispatch_safehold() {
    if (control_cycle_count - last_checkin_cycle_fp->get() <= PAN::one_day_ccno) {
        transition_to(fault_checker_state_t::unfaulted);
        return mission_state_t::standby;
    }
    else {
        return mission_state_t::safehold;
    }
}
