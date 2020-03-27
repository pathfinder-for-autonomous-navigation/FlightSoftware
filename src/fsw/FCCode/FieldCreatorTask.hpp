#ifndef FIELD_CREATOR_TASK_HPP_
#define FIELD_CREATOR_TASK_HPP_

#include "ControlTask.hpp"
#include <common/Fault.hpp>

#include <adcs/havt_devices.hpp> // needed for ADCSCommander fill-in

// This class does the unpleasant task of creating state fields that
// controllers expect to see but for which we haven't defined any
// behavior yet.
//
// As flight software develops, this list will grow longer and shorter, but
// eventually become zero.
class FieldCreatorTask : public ControlTask<void> {
    public:
      ReadableStateField<lin::Vector3d> pos_f;
      ReadableStateField<lin::Vector3d> pos_baseline_f;

      ReadableStateField<unsigned char> prop_state_f;
      Fault failed_pressurize_f;
      Fault overpressured_f;

      FieldCreatorTask(StateFieldRegistry& r) : 
        ControlTask<void>(r),
        pos_f("orbit.pos", Serializer<lin::Vector3d>(0,100000,100)),
        pos_baseline_f("orbit.baseline_pos", Serializer<lin::Vector3d>(0,100000,100)),
        prop_state_f("prop.state", Serializer<unsigned char>(1)),
        failed_pressurize_f("prop.failed_pressurize", 1, TimedControlTaskBase::control_cycle_count),
        overpressured_f("prop.overpressured", 1, TimedControlTaskBase::control_cycle_count)
      {
          // Create the fields!

          // For AttitudeComputer
          add_readable_field(pos_f);
          add_readable_field(pos_baseline_f);

          // For propulsion controller
          add_readable_field(prop_state_f);
          add_fault(failed_pressurize_f);
          add_fault(overpressured_f);
      }

      void execute() {
          // Do nada
      }

      ~FieldCreatorTask() {}
};

#endif
