#ifndef DUMMY_MAIN_CONTROL_LOOP_TASK_HPP_
#define DUMMY_MAIN_CONTROL_LOOP_TASK_HPP_

#include "ControlTask.hpp"
#include "DebugTask.hpp"
#include <common/StateField.hpp>
#include <common/StateFieldRegistry.hpp>

class DummyMainControlLoop : public ControlTask<void> {
   protected:
    DebugTask debug_task;

    Serializer<unsigned int> value_sr;
    ReadableStateField<unsigned int> readable_f;
    WritableStateField<unsigned int> writable_f;

   public:
    /**
     * @brief Construct a new Main Control Loop Task object
     * 
     * @param registry State field registry
     */
    DummyMainControlLoop(StateFieldRegistry& registry);

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
