#ifndef DUMMY_MAIN_CONTROL_LOOP_TASK_HPP_
#define DUMMY_MAIN_CONTROL_LOOP_TASK_HPP_

#include <fsw/FCCode/ControlTask.hpp>
#include <fsw/FCCode/DebugTask.hpp>
#include <common/StateField.hpp>
#include <common/StateFieldRegistry.hpp>

class DummyMainControlLoop : public ControlTask<void> {
   protected:
    DebugTask debug_task;

    Serializer<uint32_t> value_sr;
    ReadableStateField<uint32_t> readable_f;
    WritableStateField<uint32_t> writable_f;

   public:
    /**
     * @brief Construct a new Main Control Loop Task object
     * 
     * @param registry State field registry
     */
    explicit DummyMainControlLoop(StateFieldRegistry& registry);

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
