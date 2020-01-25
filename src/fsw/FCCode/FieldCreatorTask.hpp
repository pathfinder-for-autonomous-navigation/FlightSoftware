#ifndef FIELD_CREATOR_TASK_HPP_
#define FIELD_CREATOR_TASK_HPP_

#include "ControlTask.hpp"

#include <adcs/adcs_havt_devices.hpp> // needed for ADCSCommander fill-in

// This class does the unpleasant task of creating state fields that
// controllers expect to see but for which we haven't defined any
// behavior yet.
//
// As flight software develops, this list will grow longer and shorter, but
// eventually become zero.
class FieldCreatorTask : public ControlTask<void> {
    public:
      ReadableStateField<d_vector_t> pos_f;
      ReadableStateField<d_vector_t> pos_baseline_f;

      ReadableStateField<unsigned char> prop_state_f;

      FieldCreatorTask(StateFieldRegistry& r) : 
        ControlTask<void>(r),
        pos_f("orbit.pos", Serializer<d_vector_t>(0,100000,100)),
        pos_baseline_f("orbit.baseline_pos", Serializer<d_vector_t>(0,100000,100)),
        prop_state_f("prop.state", Serializer<unsigned char>(1))
      {
          // Create the fields!

          // For AttitudeComputer
          add_readable_field(pos_f);
          add_readable_field(pos_baseline_f);

          // For propulsion controller
          add_readable_field(prop_state_f);

          // For ADCS Controller
          add_writable_field(rwa_mode_f);
          add_writable_field(rwa_speed_cmd_f);
          add_writable_field(rwa_torque_cmd_f);
          add_writable_field(rwa_speed_filter_f);
          add_writable_field(rwa_ramp_filter_f);
          add_writable_field(mtr_mode_f);
          add_writable_field(mtr_cmd_f);
          add_writable_field(mtr_limit_f);
          add_writable_field(ssa_voltage_filter_f);
          add_writable_field(imu_mode_f);
          add_writable_field(imu_mag_filter_f);
          add_writable_field(imu_gyr_filter_f);
          add_writable_field(imu_gyr_temp_filter_f);
          add_writable_field(imu_gyr_temp_kp_f);
          add_writable_field(imu_gyr_temp_ki_f);
          add_writable_field(imu_gyr_temp_kd_f);
          add_writable_field(imu_gyr_temp_desired_f);

          // reserve memory
          havt_cmd_table_vector_f.reserve(adcs_havt::Index::_LENGTH);
          // fill vector of statefields for cmd havt
          char buffer[50];
          for (unsigned int idx = adcs_havt::Index::IMU_GYR; idx < adcs_havt::Index::_LENGTH; idx++ )
          {
            std::memset(buffer, 0, sizeof(buffer));
            sprintf(buffer,"adcs_cmd.havt_device");
            sprintf(buffer + strlen(buffer), "%u", idx);
            havt_cmd_table_vector_f.emplace_back(buffer, havt_bool_sr);
            add_writable_field(havt_cmd_table_vector_f[idx]);
          }
      }

      void execute() {
          // Do nada
      }

      ~FieldCreatorTask() {}
};

#endif
