#include "DockingController.hpp"

DockingController::DockingController(StateFieldRegistry& registry)
    : ControlTask<void>(registry), step_angle_sr(0, 10, 10), step_angle_f("step_angle", step_angle_sr),
    is_turning_clockwise_sr(), is_turning_clockwise_f("is_turning_clockwise", is_turning_clockwise_sr),
    is_enabled_sr(), is_enabled_f("is_enabled", is_enabled_sr)
    {
        step_angle_fp = find_writable_field<unsigned int>("step_angle", __FILE__, __LINE__);
        add_writable_field(step_angle_f);

        is_turning_clockwise_fp = find_writable_field<unsigned int>("is_turning_clockwise", __FILE__, __LINE__);
        add_readable_field(is_turning_clockwise_f);

        is_enabled_fp = find_writable_field<unsigned int>("is_enabled", __FILE__, __LINE__);
        add_readable_field(is_enabled_f);
    }

    void execute() { 
      step_angle_f.set(step_angle_fp->get());
      is_turning_f_clockwise.set(is_turning_clockwise_fp.get());
      is_enabled_f.set(is_enabled_fp.get());
    }