#ifndef FIELD_CREATOR_TASK_HPP_
#define FIELD_CREATOR_TASK_HPP_

#include "ControlTask.hpp"
#include <common/Fault.hpp>

#include <adcs/havt_devices.hpp> // needed for ADCSCommander fill-in
#include <gnc/constants.hpp>

// This class does the unpleasant task of creating state fields that
// controllers expect to see but for which we haven't defined any
// behavior yet.
//
// As flight software develops, this list will grow longer and shorter, but
// eventually become zero.
class FieldCreatorTask : public ControlTask<void> {
    public:
      ReadableStateField<double> time_f;
      ReadableStateField<lin::Vector3d> pos_f;
      ReadableStateField<lin::Vector3d> vel_f;
      ReadableStateField<lin::Vector3d> pos_baseline_f;
      ReadableStateField<lin::Vector3d> vel_baseline_f;
      ReadableStateField<unsigned char> prop_planner_state_f;
      ReadableStateField<unsigned int> bootcount_f;

      FieldCreatorTask(StateFieldRegistry& r) : 
        ControlTask<void>(r),
        time_f("orbit.time", Serializer<double>(0.0, 18'446'744'073'709'551'616.0, 64)),
        pos_f("orbit.pos", Serializer<lin::Vector3d>(0,100000,100)),
        vel_f("orbit.vel", Serializer<lin::Vector3d>(0,100000,100)),
        pos_baseline_f("orbit.baseline_pos", Serializer<lin::Vector3d>(0,100000,100)),
        vel_baseline_f("orbit.baseline_vel", Serializer<lin::Vector3d>(0,100000,100)),
        prop_planner_state_f("prop.planner.state", Serializer<unsigned char>(2)),
        bootcount_f("pan.bootcount",Serializer<unsigned int>(0xfffffff), 1000)
      {
          // For OrbitController
          add_readable_field(time_f); // Time since the PAN epoch in seconds
          add_readable_field(pos_f);
          add_readable_field(vel_f);
          add_readable_field(pos_baseline_f);
          add_readable_field(vel_baseline_f);

          constexpr double nan_d = gnc::constant::nan;
          pos_f.set({nan_d, nan_d, nan_d});
          vel_f.set({nan_d, nan_d, nan_d});
          pos_baseline_f.set({nan_d, nan_d, nan_d});
          vel_baseline_f.set({nan_d, nan_d, nan_d});

          add_readable_field(prop_planner_state_f);
          prop_planner_state_f.set(0);

          add_readable_field(bootcount_f);
      }

      void execute() {
          // Do nada
      }

      ~FieldCreatorTask() {}
};

#endif
