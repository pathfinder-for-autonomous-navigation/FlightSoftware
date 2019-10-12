#ifndef MISSION_MANAGER_HPP_
#define MISSION_MANAGER_HPP_

#include <ControlTask.hpp>

class ClockManager : public ControlTask<void> {
   public:
    ClockManager(StateFieldRegistry& registry);
    void execute() override;

   protected:
    Serializer<unsigned int> control_cycle_count_sr;
    ReadableStateField<unsigned int> control_cycle_count_f;
};

#endif
