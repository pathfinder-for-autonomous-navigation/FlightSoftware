#ifndef DOCKING_CONTROLLER_HPP_
#define DOCKING_CONTROLLER_HPP_

#include "../../test/StateFieldRegistryMock.hpp"
#include <ControlTask.hpp>
#include <unity.h>
//include the Device Class
// I need to create a pointer to the input "Is Docked" and a field for output set ""

class DockingController : public ControlTask<void> {
   public:

    DockingController(StateFieldRegistry& registry);
    void execute();

   protected:
    void set_step_angle();
    

    Serializer<unsigned int> set_mode_sr;
    WritableStateField<unsigned int>set_mode_f;

    std::shared_ptr<WritableStateField<unsigned int>> check_mode_fp;

    /**
    std::shared_ptr<WritableStateField<float>> step_angle_fp;
    Serializer<float> step_angle_sr;
    WritableStateField<float>step_angle_f;

    std::shared_ptr<WritableStateField<bool>> is_turning_clockwise_fp;
    Serializer<bool> is_turning_clockwise_sr;
    ReadableStateField<bool>is_turning_clockwise_f;

    std::shared_ptr<WritableStateField<bool>> is_enabled_fp;
    Serializer<bool> is_enabled_sr;
    ReadableStateField<bool>is_enabled_f;
    **/
    
};

#endif