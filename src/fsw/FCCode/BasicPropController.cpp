#include "BasicPropController.hpp"

BasicPropController::BasicPropController(StateFieldRegistry& r, unsigned int offset) :
    TimedControlTask<void>("prop_ctrl_ct", r, offset),
    execute_schedule("prop.sched.run", Serializer<bool>()),
    valve1_schedule("prop.sched.v1", Serializer<unsigned int>(0, 1000)),
    valve2_schedule("prop.sched.v2", Serializer<unsigned int>(0, 1000)),
    valve3_schedule("prop.sched.v3", Serializer<unsigned int>(0, 1000)),
    valve4_schedule("prop.sched.v4", Serializer<unsigned int>(0, 1000)),
    intertank_1_ctrl("prop.intertank.ctrl1", Serializer<bool>()),
    intertank_2_ctrl("prop.intertank.ctrl2", Serializer<bool>())
{

}

void BasicPropController::execute() {
    
}
