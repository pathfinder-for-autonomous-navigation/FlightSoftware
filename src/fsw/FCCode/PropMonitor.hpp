#ifndef PROP_MONITOR_HPP_
#define PROP_MONITOR_HPP_

#include "TimedControlTask.hpp"

class PropMonitor : public TimedControlTask<void>() {
  public:
    PropMonitor(StateFieldRegistry& r, unsigned int offset);
    void execute() override;
  private:
    ReadableStateField<temperature_t> temp_tank1_f;
    ReadableStateField<temperature_t> temp_tank2_f;
    ReadableStateField<unsigned int> pressure_tank2_f;
}

#endif
