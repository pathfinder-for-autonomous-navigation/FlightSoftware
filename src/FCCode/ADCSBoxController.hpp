#ifndef ADCS_BOX_CONTROLLER_HPP_
#define ADCS_BOX_CONTROLLER_HPP_

#include <ADCS.hpp>
#include "TimedControlTask.hpp"

/**
 * @brief Takes input command statefields and commands the ADCS Box.
 * 
 * Note this CT doesn't do any computing, just actuation
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
    ADCSBoxController(StateFieldRegistry &registry, unsigned int offset, Devices::ADCS &_adcs);

    /** ADCS Driver. **/
    Devices::ADCS& adcs_system;

    /**
    * @brief Gets inputs from the ADCS box and dumps them into the state
    * fields listed below.
    */
    void execute() override;

protected:
    /**
    * @brief Commands to actuate on the ADCS Box
    */
   
    const WritableStateField<unsigned char>* adcs_state_fp;

    /**
     * @brief RWA commands
     * 
     */
    const WritableStateField<unsigned char>* rwa_mode_fp;
    const WritableStateField<f_vector_t>* rwa_cmd_fp;
    const WritableStateField<float>* rwa_speed_filter_fp;
    const WritableStateField<float>* rwa_ramp_filter_fp;

    /**
     * @brief MTR commands
     * 
     */
    const WritableStateField<unsigned char>* mtr_mode_fp;
    const WritableStateField<f_vector_t>* mtr_cmd_fp;
    const WritableStateField<float>* mtr_limit_fp;

    //perhaps change box monitor to a writeable statefield
    const WritableStateField<unsigned char>* ssa_mode_fp;

    const WritableStateField<float>* ssa_voltage_filter_fp;

    /**
     * @brief IMU commands
     * 
     */
    const WritableStateField<unsigned char>* imu_mode_fp;
    const WritableStateField<float>* imu_mag_filter_fp;
    const WritableStateField<float>* imu_gyr_filter_fp;
    const WritableStateField<float>* imu_gyr_temp_filter_fp;
    const WritableStateField<float>* imu_gyr_temp_kp;
    const WritableStateField<float>* imu_gyr_temp_ki;
    const WritableStateField<float>* imu_gyr_temp_kd;
    const WritableStateField<float>* imu_gyr_temp_desired;
};

#endif
