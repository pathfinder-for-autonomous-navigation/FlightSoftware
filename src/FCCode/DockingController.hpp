#ifndef DOCKING_CONTROLLER_HPP_
#define DOCKING_CONTROLLER_HPP_

#include "../../test/StateFieldRegistryMock.hpp"
#include <ControlTask.hpp>
#include <unity.h>
#include "../../lib/Drivers/DockingSystem.cpp"

class DockingController : public ControlTask<void> {
   public:
    DockingSystem& docksys;
    DockingController(StateFieldRegistry& registry);
    void execute() override;

   protected:

    std::shared_ptr<WritableStateField<unsigned int>> set_mode_fp;

    Serializer<unsigned int> check_mode_sr;
    WritableStateField<unsigned int>check_mode_f;

};

#endif