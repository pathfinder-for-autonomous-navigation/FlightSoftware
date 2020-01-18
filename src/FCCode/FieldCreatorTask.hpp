#ifndef FIELD_CREATOR_TASK_HPP_
#define FIELD_CREATOR_TASK_HPP_

#include "ControlTask.hpp"

// This class does the unpleasant task of creating state fields that
// controllers expect to see but for which we haven't defined any
// behavior yet.
//
// As flight software develops, this list will grow longer and shorter, but
// eventually become zero.
class FieldCreatorTask : public ControlTask<void> {
    public:
      WritableStateField<f_quat_t> adcs_cmd_attitude_f;
      WritableStateField<float> adcs_ang_rate_f;
      WritableStateField<float> adcs_min_stable_ang_rate_f;

      ReadableStateField<d_vector_t> pos_f;
      ReadableStateField<d_vector_t> pos_baseline_f;

      WritableStateField<bool> docking_config_cmd_f;

      ReadableStateField<unsigned char> prop_mode_f;

      // begin fields necessary for adcs_box controller
      const WritableStateField<unsigned char> rwa_mode_fp;
      const WritableStateField<f_vector_t> rwa_cmd_fp;
      const WritableStateField<float> rwa_speed_filter_fp;
      const WritableStateField<float> rwa_ramp_filter_fp;

      const WritableStateField<unsigned char> mtr_mode_fp;
      const WritableStateField<f_vector_t> mtr_cmd_fp;
      const WritableStateField<float> mtr_limit_fp;

      const WritableStateField<unsigned char> ssa_mode_fp;
      const WritableStateField<float> ssa_voltage_filter_fp;

      const WritableStateField<unsigned char> imu_mode_fp;
      const WritableStateField<float> imu_mag_filter_fp;
      const WritableStateField<float> imu_gyr_filter_fp;
      const WritableStateField<float> imu_gyr_temp_filter_fp;
      const WritableStateField<float> imu_gyr_temp_kp_fp;
      const WritableStateField<float> imu_gyr_temp_ki_fp;
      const WritableStateField<float> imu_gyr_temp_kd_fp;
      const WritableStateField<float> imu_gyr_temp_desired_fp;

      std::vector<WritableStateField<bool>> havt_cmd_table_vector_fp;
      }
      // end fields necessary for adcs_box controller

      // rwa_mode_fp = find_writable_field<unsigned char>("adcs_cmd.rwa_cmd", __FILE__, __LINE__);
      // rwa_cmd_fp = find_writable_field<f_vector_t>("adcs_cmd.rwa_cmd", __FILE__, __LINE__);
      // rwa_speed_filter_fp = find_writable_field<float>("adcs_cmd.rwa_speed_filter", __FILE__, __LINE__);
      // rwa_ramp_filter_fp = find_writable_field<float>("adcs_cmd.rwa_ramp_filter", __FILE__, __LINE__);

      // mtr_mode_fp = find_writable_field<unsigned char>("adcs_cmd.mtr_mode", __FILE__, __LINE__);
      // mtr_cmd_fp = find_writable_field<f_vector_t>("adcs_cmd.mtr_cmd", __FILE__, __LINE__);
      // mtr_limit_fp = find_writable_field<float>("adcs_cmd.mtr_limit", __FILE__, __LINE__);

      // ssa_mode_fp = find_writable_field<unsigned char>("adcs_cmd.ssa_mode", __FILE__, __LINE__);
      // ssa_voltage_filter_fp = find_writable_field<float>("adcs_cmd.ssa_voltage_filter", __FILE__, __LINE__);

      // imu_mode_fp = find_writable_field<unsigned char>("adcs_cmd.imu_mode", __FILE__, __LINE__);
      // imu_mag_filter_fp = find_writable_field<float>("adcs_cmd.imu_mag_filter", __FILE__, __LINE__);
      // imu_gyr_filter_fp = find_writable_field<float>("adcs_cmd.imu_gyr_filter", __FILE__, __LINE__);
      // imu_gyr_temp_filter_fp = find_writable_field<float>("adcs_cmd.imu_gyr_temp_filter", __FILE__, __LINE__);
      // imu_gyr_temp_kp_fp = find_writable_field<float>("adcs_cmd.imu_temp_kp", __FILE__, __LINE__);
      // imu_gyr_temp_ki_fp = find_writable_field<float>("adcs_cmd.imu_temp_ki", __FILE__, __LINE__);
      // imu_gyr_temp_kd_fp = find_writable_field<float>("adcs_cmd.imu_temp_kd", __FILE__, __LINE__);
      // imu_gyr_temp_desired_fp = find_writable_field<float>("adcs_cmd.imu_gyr_temp_desired", __FILE__, __LINE__);

      FieldCreatorTask(StateFieldRegistry& r) : 
        ControlTask<void>(r),
        adcs_cmd_attitude_f("adcs.cmd_attitude", Serializer<f_quat_t>()),
        adcs_ang_rate_f("adcs.ang_rate", Serializer<float>(0, 10, 4)),
        adcs_min_stable_ang_rate_f("adcs.min_stable_ang_rate", Serializer<float>(0, 10, 4)),
        pos_f("orbit.pos", Serializer<d_vector_t>(0,100000,100)),
        pos_baseline_f("orbit.baseline_pos", Serializer<d_vector_t>(0,100000,100)),
        docking_config_cmd_f("docksys.config_cmd", Serializer<bool>()),
        prop_mode_f("prop.mode", Serializer<unsigned char>(1))
        
      {
          // Create the fields!

          // For MissionManager
          add_writable_field(adcs_cmd_attitude_f);
          add_writable_field(adcs_ang_rate_f);
          add_writable_field(adcs_min_stable_ang_rate_f);

          // For AttitudeComputer
          add_readable_field(pos_f);
          add_readable_field(pos_baseline_f);

          // For DockingController
          add_writable_field(docking_config_cmd_f);

          // For propulsion controller
          add_readable_field(prop_mode_f);
      }

      void execute() {
          // Do nada
      }

      ~FieldCreatorTask() {}
};

#endif
