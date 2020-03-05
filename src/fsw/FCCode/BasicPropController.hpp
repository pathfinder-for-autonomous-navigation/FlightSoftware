#ifndef BASIC_PROP_CONTROLLER_HPP_
#define BASIC_PROP_CONTROLLER_HPP_

#include "TimedControlTask.hpp"

class BasicPropController : public TimedControlTask<void> {
  public:
    BasicPropController(StateFieldRegistry& r, unsigned int offset);
    void execute() override;

  protected:
    WritableStateField<bool> execute_schedule;
    WritableStateField<unsigned int> valve1_schedule;
    WritableStateField<unsigned int> valve2_schedule;
    WritableStateField<unsigned int> valve3_schedule;
    WritableStateField<unsigned int> valve4_schedule;

    WritableStateField<bool> intertank_1_ctrl;
    WritableStateField<bool> intertank_2_ctrl;
};

#endif
