#ifndef DOCKING_CONTROLLER_HPP_
#define DOCKING_CONTROLLER_HPP_

#include "../../test/StateFieldRegistryMock.hpp"
#include <ControlTask.hpp>
#include <unity.h>
#include "../../lib/Drivers/DockingSystem.cpp"

class DockingController : public ControlTask<void> {
   public:
    
    DockingController(StateFieldRegistry& registry, DockingSystem& docksys);
    void execute() override;

   protected:

    std::shared_ptr<WritableStateField<unsigned int>> is_turning_fp;
    Serializer<unsigned int> is_turning_sr;
    WritableStateField<unsigned int>is_turning_f;

    std::shared_ptr<WritableStateField<unsigned int>> docking_mode_fp;
    Serializer<unsigned int> docking_mode_sr;
    WritableStateField<unsigned int>docking_mode_f;

};

#endif