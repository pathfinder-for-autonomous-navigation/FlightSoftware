#ifndef DOCKING_CONTROLLER_HPP_
#define DOCKING_CONTROLLER_HPP_

#include "../../test/StateFieldRegistryMock.hpp"
#include <ControlTask.hpp>
#include <unity.h>

class DockingController : public ControlTask<void> {
   public:

    DockingController(StateFieldRegistry& registry);
    void execute();

   protected:
    void set_step_angle();
    

    std::shared_ptr<WritableStateField<float>> step_angle_fp;
    Serializer<float> step_angle_sr;
    WritableStateField<float>step_angle;

    std::shared_ptr<WritableStateField<bool>> is_turning_clockwise_fp;
    Serializer<bool> is_turning_clockwise_sr;
    ReadableStateField<bool>is_turning_clockwise;

    std::shared_ptr<WritableStateField<float>> is_enabled_fp;
    Serializer<bool> is_enabled_sr;
    ReadableStateField<bool>is_enabled;
    
};

#endif