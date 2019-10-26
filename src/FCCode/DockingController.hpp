#ifndef DOCKING_CONTROLLER_HPP_
#define DOCKING_CONTROLLER_HPP_

#include "../../test/StateFieldRegistryMock.hpp"
#include <ControlTask.hpp>
#include <unity.h>

class DockingController : public ControlTask<void> {
   public:
    // construct a new docking controller object
    DockingController(StateFieldRegistry& registry);

    void execute();

   protected:
    Serializer<float> step_angle_sr;
    WritableStateField<float>step_angle;

    Serializer<bool> is_turning_clockwise_sr;
    ReadableStateField<bool>is_turning_clockwise;

    Serializer<bool> is_enabled_sr;
    ReadableStateField<bool>is_enabled;
    
};

#endif