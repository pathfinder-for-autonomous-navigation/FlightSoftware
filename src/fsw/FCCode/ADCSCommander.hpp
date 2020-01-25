#ifndef ADCS_COMMANDER_HPP_
#define ADCS_COMMANDER_HPP_

#include "TimedControlTask.hpp"

/**
 * @brief Given a desired pointing target, and other objectives, generate ADCS commands to apply
 * 
 */
class ADCSCommander : public TimedControlTask<void> {
   public:
    /**
     * @brief Construct a new ADCSCommander.
     * 
     * @param registry 
     */
    ADCSCommander(StateFieldRegistry& registry, unsigned int offset);

    /**
     * @brief Determine control outputs for the ADCSBoxController
     */
    void execute() override;

   protected:
    // eventually everything else necessary for generating commands
    // especially all the gnc stuff

    // field pointer for adcs_montor device;
    std::vector<WritableStateField<bool>*> havt_read_table_vector_fp;

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
};

#endif
