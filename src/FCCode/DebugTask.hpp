#ifndef DEBUG_TASK_HPP_
#define DEBUG_TASK_HPP_

#include <ControlTask.hpp>

class DebugTask : public ControlTask<void> {
   public:
    /**
     * @brief Construct a new Debug Task object
     * 
     * @param registry 
     */
    DebugTask(StateFieldRegistry& registry);

    /**
     * @brief Runs the debug task (processes state field commands present in the serial buffer.)
     */
    void execute() override;
    
    /**
     * @brief Initializes the debug console.
     */
    void init();
};

#endif
