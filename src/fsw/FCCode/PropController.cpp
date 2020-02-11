#include <fsw/FCCode/PropController.hpp>

using namespace Devices;

PropController::PropController(StateFieldRegistry& registry, unsigned int offset)
: TimedControlTask<void>(registry, "prop", offset),
  prop_state_f("prop.state", Serializer<unsigned int>(6)),
  fire_cycle_f("prop.fire_cycle", Serializer<unsigned int>(256)),
  sched_valve1_f("prop.sched_valve1", Serializer<unsigned int>(999)),
  sched_valve2_f("prop.sched_valve2", Serializer<unsigned int>(999)),
  sched_valve3_f("prop.sched_valve3", Serializer<unsigned int>(999)),
  sched_valve4_f("prop.sched_valve4", Serializer<unsigned int>(999))
{

  add_writable_field(prop_state_f);
  add_writable_field(fire_cycle_f);
  add_writable_field(sched_valve1_f);
  add_writable_field(sched_valve2_f);
  add_writable_field(sched_valve3_f);
  add_writable_field(sched_valve4_f);
}

void PropController::execute()
{
  switch( static_cast<prop_state_t>(prop_state_f.get()) )
  {
    case prop_state_t::disabled:
      return dispatch_disabled();
    case prop_state_t::idle:
      return dispatch_idle();
    case prop_state_t::pressurizing:
      return dispatch_pressurizing();
    case prop_state_t::await_firing:
      return dispatch_await_firing();
    case prop_state_t::firing:
      return dispatch_firing();
    case prop_state_t::venting:
      return dispatch_venting();
    case prop_state_t::handling_fault:
      return dispatch_handling_fault();
    default:
      return dispatch_handling_fault();
      // TODO: trigger some sort of complaint
  }
}
void PropController::set_schedule(unsigned int v1, unsigned int v2, unsigned int v3, unsigned int v4, unsigned int ctrl_cycles_from_now)
{

}

void PropController::dispatch_disabled()
{
  if (!assert_state(prop_state_t::disabled))
    return;
}

void PropController::dispatch_idle()
{
  if (!assert_state(prop_state_t::idle))
    return;
}

void PropController::dispatch_pressurizing()
{
  if (!assert_state(prop_state_t::pressurizing))
    return;
  if (!is_at_threshold_pressure())
    change_state(prop_state_t::await_firing);
}

void PropController::dispatch_venting()
{
  if (!assert_state(prop_state_t::venting))
    return;
}

void PropController::dispatch_await_firing()
{
  if (!assert_state(prop_state_t::await_firing))
    return;
}

void PropController::dispatch_firing()
{
  if (!assert_state(prop_state_t::firing))
    return;
}

void PropController::dispatch_handling_fault()
{

}

void PropController::_Pressurizing::entry_protocol()
{
  // Set which Tank1 valve to use (default: valve_num = 0)
  if (should_use_backup())
    valve_num = 1;
  // Reset the pressurizing cycles count to 0
  current_cycle = 0;
}

bool PropController::_Pressurizing::should_use_backup()
{
  // TODO: determine this
  return false;
}

prop_state_t PropController::_Pressurizing::next_state()
{
  if (is_at_threshold_pressure())
  {
    prop_system.close_valve(Tank1, valve_num);
    return prop_state_t::await_firing;
  }
  // Tank2 is not at threshold pressure
  if (Tank1.is_valve_open(valve_num))
  { 
    handle_currently_pressurizing();
    return prop_state_t::pressurizing;
  }
  // Tank is not at threshold pressure and is not opened
  if (current_cycle > max_cycles)
  {
    handle_pressurize_failed();
    return prop_state_t::handling_fault;
  }
  else
  {
    start_pressurize_cycle();
    return prop_state_t::pressurizing;
  }
}

void PropController::_Pressurizing::handle_currently_pressurizing()
{
    // check if 1s has passed and close valve if it has
    // TODO: fix cycle_duration_ms
    const static unsigned int cycle_duration_ms = 1000/PAN::control_cycle_time_ms; // 1000 ms = 1 s
    // Assuming that this subtraction is always safe -- TODO: confirm this?
    if (control_cycle_count - cycle_start_time > cycle_duration_ms)
    {
      prop_system.close_valve(Tank1, valve_num);
    }
}

void PropController::_Pressurizing::handle_pressurize_failed()
{
  // TODO need to set some sort of fault
}

void PropController::_Pressurizing::start_pressurize_cycle()
{
  // Make the start of this cycle
  cycle_start_time = control_cycle_count;
  // Increment the cycle count
  current_cycle++;
  // Open the valve
  prop_system.open_valve(Tank1, valve_num);
}

void PropController::_Pressurizing::exit_protocol()
{

}

bool PropController::_Pressurizing::assert_state()
{
  return false;
}