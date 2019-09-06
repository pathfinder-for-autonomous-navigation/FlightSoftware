#ifndef MAIN_CONTROL_LOOP_TASK_HPP_
#define MAIN_CONTROL_LOOP_TASK_HPP_

#include "ControlTask.hpp"
#include "DebugTask.hpp"
#include "StateField.hpp"
#include "StateFieldRegistry.hpp"

class MainControlLoopTask : public ControlTask<void> {
   protected:
    DebugTask debug_task;

    std::shared_ptr<ReadableStateField<signed int>> tank_inner_temperature;
    std::shared_ptr<ReadableStateField<signed int>> tank_outer_temperature;
    std::shared_ptr<WritableStateField<gps_time_t>> firing_time;

   public:
    MainControlLoopTask(StateFieldRegistry& registry);
    void execute() override;
};

#endif