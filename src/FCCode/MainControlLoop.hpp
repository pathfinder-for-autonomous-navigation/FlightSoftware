#ifndef MAIN_CONTROL_LOOP_TASK_HPP_
#define MAIN_CONTROL_LOOP_TASK_HPP_

#include <ControlTask.hpp>
#include "DebugTask.hpp"
#include <StateField.hpp>
#include <StateFieldRegistry.hpp>

class MainControlLoop : public ControlTask<void> {
   protected:
    DebugTask debug_task;

    std::shared_ptr<ReadableStateField<signed int>> tank_inner_temperature;
    std::shared_ptr<WritableStateField<signed int>> tank_outer_temperature;

   public:
    /**
     * @brief Construct a new Main Control Loop Task object
     * 
     * @param registry State field registry
     */
    MainControlLoop(StateFieldRegistry& registry);

    /**
     * @brief Initializes debug console.
     */
    void init();

    /**
     * @brief Processes state field commands present in the serial buffer.
     */
    void execute() override;
};

#endif
