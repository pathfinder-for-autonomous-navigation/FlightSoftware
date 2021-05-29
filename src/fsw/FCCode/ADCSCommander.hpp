#ifndef ADCS_COMMANDER_HPP_
#define ADCS_COMMANDER_HPP_

#include "TimedControlTask.hpp"

/**
 * @brief Given a desired pointing target, and other objectives,
 * generate ADCS commands to apply
 * 
 */
class ADCSCommander : public TimedControlTask<void> {
   public:
    /**
     * @brief Construct a new ADCSCommander.
     * 
     * @param registry 
     */
    ADCSCommander(StateFieldRegistry& registry);

    /**
     * @brief Determine control outputs for the ADCSBoxController
     */
    void execute() override;

   protected:
    // input fields, given by a casted adcs_state_t enum
    const WritableStateField<unsigned char>* adcs_state_fp;

    // outputs from AttitudeController as inputs
    const WritableStateField<lin::Vector3f>* pointer_rwa_torque_cmd;
    const WritableStateField<lin::Vector3f>* pointer_mtr_cmd;

    // begin output fields necessary for adcs_box controller
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

    WritableStateField<unsigned char> mag1_mode_f;
    WritableStateField<unsigned char> mag2_mode_f;

    WritableStateField<float> imu_mag_filter_f;
    WritableStateField<float> imu_gyr_filter_f;
    WritableStateField<float> imu_gyr_temp_filter_f;

    WritableStateField<unsigned char> imu_gyr_temp_pwm_f;
    WritableStateField<float> imu_gyr_temp_desired_f;

    Serializer<bool> bool_sr;
    std::vector<WritableStateField<bool>> havt_cmd_reset_vector_f;
    std::vector<WritableStateField<bool>> havt_cmd_disable_vector_f;

    template<typename T, size_t N>
    std::array<T, N> lin_to_std(lin::Vector<T,N> v){
        std::array<T,N> ret;
        for(unsigned char i = 0; i<N; i++){
            ret[i] = v(i);
        }
        return ret;
    }   
};

#endif
