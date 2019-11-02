#pragma once
#include <ControlTask.hpp>
class QuakeManager : public ControlTask<void> {
   public:
    QuakeManager(StateFieldRegistry& registry);
    void execute() override;

   protected:
   void dispatch_startup();
      /**
     * @brief Control cycle count, provided by ClockManager.
     */
    std::shared_ptr<ReadableStateField<unsigned int>> control_cycle_count_fp;

};