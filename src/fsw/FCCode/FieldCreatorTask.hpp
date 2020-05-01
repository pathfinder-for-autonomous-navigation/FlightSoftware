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
      ReadableStateField<double> time_f;
      ReadableStateField<lin::Vector3d> pos_baseline_f;
      ReadableStateField<lin::Vector3d> pos_f;
      ReadableStateField<lin::Vector3d> pos_baseline_f;

      FieldCreatorTask(StateFieldRegistry& r) : 
        ControlTask<void>(r),
        time_f("orbit.time", Serializer<double>()),
        pos_f("orbit.pos", Serializer<lin::Vector3d>(0,100000,100)),
        pos_baseline_f("orbit.baseline_pos", Serializer<lin::Vector3d>(0,100000,100))
      {
          // Create the fields!

          // For AttitudeComputer
          add_readable_field(time_f); // Time since the PAN epoch in seconds
          add_readable_field(pos_f);
          add_readable_field(pos_baseline_f);

      }

      void execute() {
          // Do nada
      }

      ~FieldCreatorTask() {}
};

#endif
