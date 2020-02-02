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
void PropController::execute()
{

}

void PropController::set_schedule(unsigned int v1, unsigned int v2, unsigned int v3, unsigned int v4, unsigned int ctrl_cycles_from_now)
{

}