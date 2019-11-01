#ifndef DOCKING_CONTROLLER_HPP_
#define DOCKING_CONTROLLER_HPP_

#include "../../test/StateFieldRegistryMock.hpp"
#include <ControlTask.hpp>
#include <unity.h>
#include <Device.hpp>
//include the Device Class

class DockingController : public ControlTask<void> {
   public:

    DockingController(StateFieldRegistry& registry);
    void execute();

   protected:
    void set_step_angle();
    

    Serializer<unsigned int> set_mode_sr;
    WritableStateField<unsigned int>set_mode_f;

    std::shared_ptr<WritableStateField<unsigned int>> check_mode_fp;
    
};

#endif