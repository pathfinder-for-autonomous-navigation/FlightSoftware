#include <fsw/FCCode/PropController.hpp>

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

unsigned int PropController::_fire_cycle = 0;

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
  // TODO 
}

bool PropController::_PropIdle::entry_protocol()
{
  // do nothing - maybe check for hardware faults?
  return true;
}

prop_state_t PropController::_PropIdle::next_state()
{
  // check schedule

  // if there is a feasible schedule then check time

  // if it is time to pressurize then transition to pressurizing

  // otherwise, stay in idle
  return prop_state_t::idle;
}

bool PropController::_Pressurizing::entry_protocol()
{
  // Set which Tank1 valve to use (default: valve_num = 0)
  if (should_use_backup())
    valve_num = 1;
  // Reset the pressurizing cycles count to 0
  current_cycle = 0;
  return true;
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
    PropulsionSystem.close_valve(Tank1, valve_num);
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
      PropulsionSystem.close_valve(Tank1, valve_num);
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
  PropulsionSystem.open_valve(Tank1, valve_num);
}

bool PropController::_Firing::entry_protocol()
{
  // Initialize the IntervalTimer
  PropulsionSystem.start_firing();
  return true;
}

prop_state_t PropController::_Firing::next_state()
{
  // Check the schedule to see if we are done
  if (is_schedule_empty())
    return prop_state_t::idle;
  else
    return prop_state_t::firing;
}

bool PropController::_Firing::is_schedule_empty()
{
  unsigned int remain = 0;
  for (size_t i = 0; i < 4; ++i)
    remain += Tank2.get_schedule_at(i);
  return remain == 0;
}

bool PropController::_Await_Firing::entry_protocol()
{
  // tank should be pressurized
  if (!PropController::is_at_threshold_pressure())
    return false;
  // there should be a schedule

  // there should be a firing time

  // we don't check for the correctness of this
  
  return true;

  // if any of these are not true then throw fault
}

prop_state_t PropController::_Await_Firing::next_state()
{
  // if we are within a control cycle of firing time then fire
  if (is_time_to_fire())
    return prop_state_t::firing;
  
  return prop_state_t::await_firing;
}
