#ifndef FIELD_CREATOR_TASK_HPP_
#define FIELD_CREATOR_TASK_HPP_

#include "ControlTask.hpp"

#include <adcs/havt_devices.hpp> // needed for ADCSCommander fill-in

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

      // begin fields necessary for adcs_box controller
      const Serializer<float> filter_sr;

      WritableStateField<unsigned char> rwa_mode_f;
      WritableStateField<f_vector_t> rwa_speed_cmd_f;
      WritableStateField<f_vector_t> rwa_torque_cmd_f;
      WritableStateField<float> rwa_speed_filter_f;
      WritableStateField<float> rwa_ramp_filter_f;

      WritableStateField<unsigned char> mtr_mode_f;
      WritableStateField<f_vector_t> mtr_cmd_f;
      WritableStateField<float> mtr_limit_f;

      WritableStateField<float> ssa_voltage_filter_f;

      WritableStateField<unsigned char> imu_mode_f;
      WritableStateField<float> imu_mag_filter_f;
      WritableStateField<float> imu_gyr_filter_f;
      WritableStateField<float> imu_gyr_temp_filter_f;

      const Serializer<float> k_sr;

      WritableStateField<float> imu_gyr_temp_kp_f;
      WritableStateField<float> imu_gyr_temp_ki_f;
      WritableStateField<float> imu_gyr_temp_kd_f;
      WritableStateField<float> imu_gyr_temp_desired_f;

      Serializer<bool> havt_bool_sr;
      std::vector<WritableStateField<bool>> havt_cmd_table_vector_f;

      FieldCreatorTask(StateFieldRegistry& r) : 
        ControlTask<void>(r),
        pos_f("orbit.pos", Serializer<d_vector_t>(0,100000,100)),
        pos_baseline_f("orbit.baseline_pos", Serializer<d_vector_t>(0,100000,100)),
        prop_state_f("prop.state", Serializer<unsigned char>(1)),
        //eventually you can move all these into ADCSCommander.cpp
        filter_sr(0,1,8),
        rwa_mode_f("adcs_cmd.rwa_mode", Serializer<unsigned char>(2)),
        rwa_speed_cmd_f("adcs_cmd.rwa_speed_cmd", Serializer<f_vector_t>(adcs::rwa::min_speed_command,adcs::rwa::max_speed_command, 16*3)),
        rwa_torque_cmd_f("adcs_cmd.rwa_torque_cmd", Serializer<f_vector_t>(adcs::rwa::min_torque, adcs::rwa::max_torque, 16*3)),
        rwa_speed_filter_f("adcs_cmd.rwa_speed_filter", filter_sr),
        rwa_ramp_filter_f("adcs_cmd.rwa_ramp_filter", filter_sr),
        mtr_mode_f("adcs_cmd.mtr_mode", Serializer<unsigned char>(2)),
        mtr_cmd_f("adcs_cmd.mtr_cmd", Serializer<f_vector_t>(adcs::mtr::min_moment, adcs::mtr::max_moment, 16*3)),
        mtr_limit_f("adcs_cmd.mtr_limit", Serializer<float>(adcs::mtr::min_moment, adcs::mtr::max_moment, 16)),
        ssa_voltage_filter_f("adcs_cmd.ssa_voltage_filter", filter_sr),
        imu_mode_f("adcs_cmd.imu_mode", Serializer<unsigned char>(4)),
        imu_mag_filter_f("adcs_cmd.imu_mag_filter", filter_sr),
        imu_gyr_filter_f("adcs_cmd.imu_gyr_filter", filter_sr),
        imu_gyr_temp_filter_f("adcs_cmd.imu_gyr_temp_filter", filter_sr),
        k_sr(std::numeric_limits<float>::min(), std::numeric_limits<float>::max(),16),
        imu_gyr_temp_kp_f("adcs_cmd.imu_temp_kp", k_sr),
        imu_gyr_temp_ki_f("adcs_cmd.imu_temp_ki", k_sr),
        imu_gyr_temp_kd_f("adcs_cmd.imu_temp_kd", k_sr),
        imu_gyr_temp_desired_f("adcs_cmd.imu_gyr_temp_desired", Serializer<float>(adcs::imu::min_eq_temp, adcs::imu::max_eq_temp, 8)),
        havt_bool_sr()
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
          havt_cmd_table_vector_f.reserve(adcs::havt::Index::_LENGTH);
          // fill vector of statefields for cmd havt
          char buffer[50];
          for (unsigned int idx = adcs::havt::Index::IMU_GYR; idx < adcs::havt::Index::_LENGTH; idx++ )
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
