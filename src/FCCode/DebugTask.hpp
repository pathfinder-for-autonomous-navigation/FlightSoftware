#ifndef DEBUG_TASK_HPP_
#define DEBUG_TASK_HPP_

#include <TimedControlTask.hpp>

class DebugTask : public TimedControlTask<void> {
   public:
    /**
     * @brief Construct a new Debug Task object
     * 
     * @param registry 
     */
    DebugTask(StateFieldRegistry& registry, unsigned int offset);

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
