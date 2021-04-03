#ifndef FIELD_CREATOR_TASK_HPP_
#define FIELD_CREATOR_TASK_HPP_

#include "ControlTask.hpp"
#include <common/Fault.hpp>

#include <adcs/havt_devices.hpp> // needed for ADCSCommander fill-in
#include <gnc/constants.hpp>

#include <common/GPSTime.hpp>

// This class does the unpleasant task of creating state fields that
// controllers expect to see but for which we haven't defined any
// behavior yet.
//
// As flight software develops, this list will grow longer and shorter, but
// eventually become zero.
class FieldCreatorTask : public ControlTask<void> {
<<<<<<< HEAD
    public:
      ReadableStateField<double> time_f;
      ReadableStateField<lin::Vector3d> pos_f;
      ReadableStateField<lin::Vector3d> vel_f;
      ReadableStateField<lin::Vector3d> pos_baseline_f;
      ReadableStateField<lin::Vector3d> vel_baseline_f;
      ReadableStateField<unsigned int> bootcount_f;

      WritableStateField<lin::Vector3d> uplink_pos_f;
      WritableStateField<lin::Vector3d> uplink_vel_f;
      WritableStateField<double> uplink_t_f;
      WritableStateField<gps_time_t> uplink_gps_t;

      FieldCreatorTask(StateFieldRegistry& r) : 
        ControlTask<void>(r),
        time_f("orbit.time", Serializer<double>(0.0, 18'446'744'073'709'551'616.0, 64)),
        pos_f("orbit.pos", Serializer<lin::Vector3d>(6771000, 6921000, 28)),
        vel_f("orbit.vel", Serializer<lin::Vector3d>(7570, 7685, 19)),
        pos_baseline_f("orbit.baseline_pos", Serializer<lin::Vector3d>(0,2000,22)),
        vel_baseline_f("orbit.baseline_vel", Serializer<lin::Vector3d>(0,11,14)),
        bootcount_f("pan.bootcount",Serializer<unsigned int>(0xfffffff), 1000),

        uplink_pos_f("orbit.uplink_pos", Serializer<lin::Vector3d>(6771000, 6921000, 28)),
        uplink_vel_f("orbit.uplink_vel", Serializer<lin::Vector3d>(7570, 7685, 19)),
        uplink_t_f("orbit.uplink_t", Serializer<double>(0.0, 18'446'744'073'709'551'616.0, 64)),
        uplink_gps_t("uplink.time", Serializer<gps_time_t>())
      {
          // For OrbitController
          add_readable_field(time_f); // Time since the PAN epoch in seconds
          add_readable_field(pos_f);
          add_readable_field(vel_f);
          add_readable_field(pos_baseline_f);
          add_readable_field(vel_baseline_f);
          add_writable_field(uplink_pos_f);
          add_writable_field(uplink_vel_f);
          add_writable_field(uplink_t_f);
          add_writable_field(uplink_gps_t);

          constexpr double nan_d = gnc::constant::nan;
          pos_f.set({nan_d, nan_d, nan_d});
          vel_f.set({nan_d, nan_d, nan_d});
          pos_baseline_f.set({nan_d, nan_d, nan_d});
          vel_baseline_f.set({nan_d, nan_d, nan_d});
          uplink_pos_f.set({nan_d, nan_d, nan_d});
          uplink_vel_f.set({nan_d, nan_d, nan_d});
=======
  public:
    ReadableStateField<unsigned int> bootcount_f;
>>>>>>> master

    FieldCreatorTask(StateFieldRegistry& r)
        : ControlTask<void>(r),
          bootcount_f("pan.bootcount",Serializer<unsigned int>(0xfffffff), 1000)
    {
        add_readable_field(bootcount_f);
    }

    ~FieldCreatorTask() = default;

    void execute()
    { }
};

#endif
