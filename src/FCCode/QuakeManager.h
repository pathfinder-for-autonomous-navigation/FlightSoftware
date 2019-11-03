#pragma once
#include <ControlTask.hpp>
#include "QuakeControlTask.h"
#include "radio_mode_t.enum"
class QuakeManager : public ControlTask<void> {
   public:
    QuakeManager(StateFieldRegistry& registry);
    void execute() override;

   protected:
   void dispatch_startup();
   void dispatch_waiting();
   void dispatch_transceiving();
   void dispatch_manual();
      /**
     * @brief Control cycle count, provided by ClockManager.
     */
    std::shared_ptr<ReadableStateField<unsigned int>> control_cycle_count_fp;
    
    Serializer<unsigned int> radio_mode_sr;
    WritableStateField<unsigned int> radio_mode_f;

    private:
    QuakeControlTask qct;

};