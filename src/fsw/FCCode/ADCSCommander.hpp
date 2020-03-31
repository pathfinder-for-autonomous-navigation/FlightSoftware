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
    ADCSCommander(StateFieldRegistry& registry, unsigned int offset);

    /**
     * @brief Determine control outputs for the ADCSBoxController
     */
    void execute() override;

   protected:
    // input fields, given by a casted adcs_state_t enum
    const WritableStateField<unsigned char>* adcs_state_fp;

    // outputs from AttitudeComputer as inputs
    const WritableStateField<lin::Vector3f>* adcs_vec1_current_fp;
    const WritableStateField<lin::Vector3f>* adcs_vec1_desired_fp;
    const WritableStateField<lin::Vector3f>* adcs_vec2_current_fp;
    const WritableStateField<lin::Vector3f>* adcs_vec2_desired_fp;

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

    WritableStateField<unsigned char> imu_mode_f;
    WritableStateField<float> imu_mag_filter_f;
    WritableStateField<float> imu_gyr_filter_f;
    WritableStateField<float> imu_gyr_temp_filter_f;

    const Serializer<float> k_sr;

    WritableStateField<float> imu_gyr_temp_kp_f;
    WritableStateField<float> imu_gyr_temp_ki_f;
    WritableStateField<float> imu_gyr_temp_kd_f;
    WritableStateField<float> imu_gyr_temp_desired_f;

    Serializer<bool> bool_sr;
    std::vector<WritableStateField<bool>> havt_cmd_reset_vector_f;
    std::vector<WritableStateField<bool>> havt_cmd_disable_vector_f;

    /** Internal specific dispatch call to calculate commands */
    void dispatch_startup();    
    void dispatch_limited();    
    void dispatch_zero_torque();
    void dispatch_zero_L();     
    void dispatch_detumble();   
    void dispatch_manual();     
    void dispatch_standby();    
    void dispatch_docking();    
};

#endif
