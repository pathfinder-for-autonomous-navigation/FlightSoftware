#ifndef ADCS_BOX_CONTROLLER_HPP_
#define ADCS_BOX_CONTROLLER_HPP_

#include "Drivers/ADCS.hpp"
#include "TimedControlTask.hpp"

/**
 * @brief Takes input command statefields and commands the ADCS Box.
 * 
 * Note this CT doesn't do any computing, just actuation
 * This CT is inteded to only do hardware calls
 */
class ADCSBoxController : public TimedControlTask<void>
{
public:
    /**
     * @brief Construct a new ADCSBoxController control task
     * 
     * @param registry input StateField registry
     * @param offset control task offset
     * @param _adcs the input adcs system
     */
    ADCSBoxController(StateFieldRegistry &registry, uint32_t offset, Devices::ADCS &_adcs);

    /** ADCS Driver. **/
    Devices::ADCS& adcs_system;

    /**
    * @brief Given the command statefields, use the ADCS driver to execute
    */
    void execute() override;

protected:
    /**
     * @brief Command to get from mission_manager
     * 
     */
    const WritableStateField<uint8_t>* adcs_state_fp = nullptr;

    /**
     * @brief RWA command fields
     * 
     */
    const WritableStateField<uint8_t>* rwa_mode_fp = nullptr;
    const WritableStateField<f_vector_t>* rwa_speed_cmd_fp = nullptr;
    const WritableStateField<f_vector_t>* rwa_torque_cmd_fp = nullptr;
    const WritableStateField<float>* rwa_speed_filter_fp = nullptr;
    const WritableStateField<float>* rwa_ramp_filter_fp = nullptr;

    /**
     * @brief MTR command fields
     * 
     */
    const WritableStateField<uint8_t>* mtr_mode_fp = nullptr;
    const WritableStateField<f_vector_t>* mtr_cmd_fp = nullptr;
    const WritableStateField<float>* mtr_limit_fp = nullptr;

    /**
     * @brief SSA command fields
     * 
     */
    const WritableStateField<uint8_t>* ssa_mode_fp = nullptr;
    const WritableStateField<float>* ssa_voltage_filter_fp = nullptr;

    /**
     * @brief IMU command fields
     * 
     */
    const WritableStateField<uint8_t>* imu_mode_fp = nullptr;
    const WritableStateField<float>* imu_mag_filter_fp = nullptr;
    const WritableStateField<float>* imu_gyr_filter_fp = nullptr;
    const WritableStateField<float>* imu_gyr_temp_filter_fp = nullptr;
    const WritableStateField<float>* imu_gyr_temp_kp_fp = nullptr;
    const WritableStateField<float>* imu_gyr_temp_ki_fp = nullptr;
    const WritableStateField<float>* imu_gyr_temp_kd_fp = nullptr;
    const WritableStateField<float>* imu_gyr_temp_desired_fp = nullptr;

    /**
     * @brief HAVT command tables, a vector of pointers to bool state fields
     * 
     */
    std::vector<const WritableStateField<bool>*> havt_cmd_reset_vector_fp;
    std::vector<const WritableStateField<bool>*> havt_cmd_disable_vector_fp;

};

#endif
