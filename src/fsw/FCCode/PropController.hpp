
#include <fsw/FCCode/TimedControlTask.hpp>
#include <fsw/FCCode/prop_state_t.enum>

class PropController : public TimedControlTask<void> {
  public:
  PropController(StateFieldRegistry& registry, unsigned int offset);
  void execute() override;

  void set_schedule(unsigned int v1, unsigned int v2, unsigned int v3, unsigned int v4, unsigned int ctrl_cycles_from_now);

  WritableStateField<unsigned int>prop_state_f;
  WritableStateField<unsigned int>fire_cycle_f;
  WritableStateField<unsigned int>sched_valve1_f;
  WritableStateField<unsigned int>sched_valve2_f;
  WritableStateField<unsigned int>sched_valve3_f;
  WritableStateField<unsigned int>sched_valve4_f;
};