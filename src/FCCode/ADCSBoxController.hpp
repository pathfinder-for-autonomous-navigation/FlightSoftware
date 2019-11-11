#ifndef ADCS_BOX_CONTROLLER_HPP_
#define ADCS_BOX_CONTROLLER_HPP_

#include <ADCS.hpp>
#include <ControlTask.hpp>

/**
  * @brief Writes outputs to the ADCS box from the values in the state fields
  * listed below.
  */
class ADCSBoxController : public ControlTask<void> {
  public:
   /**
    * @brief Construct a new ADCSBoxController.
    * 
    * @param registry
    * @param adcs ADCS Driver
    */
   ADCSBoxController(StateFieldRegistry& registry, Devices::ADCS& adcs);

   /**
    * @brief Writes outputs to the ADCS box from the values in the state fields
    * listed below.
    * 
    * It accepts box mode and torque/wheel speed/MTR commands from the Attitude Computer,
    * and does some management of the ADCS box state by asserting the filter coefficient
    * values and the gyro heater setpoint. Also does FDIR on the gyro heater.
    */
   void execute() override;

  protected:
   /** ADCS Driver. **/
   Devices::ADCS& adcs_system;

   /**
    * @brief Outputs to send to ADCS box.
    */
   //! Box mode (enabled or disabled.)
   WritableStateField<bool> adcs_box_mode_f;
   //! True if a torque command is currently being applied, false if a speed command
   //! is being applied.
   std::shared_ptr<WritableStateField<bool>> rwa_cmd_mode_fp;
   //! Torque command to apply to reaction wheels. This value is meaningless if a speed
   //! command is being applied.
   std::shared_ptr<ReadableStateField<f_vector_t>> rwa_torque_cmd_vec_fp;
   //! Speed command to apply to reaction wheels. This value is meaningless if a torque
   //! command is being applied.
   std::shared_ptr<ReadableStateField<f_vector_t>> rwa_speed_cmd_vec_fp;
   //! Magnetic moment to apply on the magnetorquers.
   std::shared_ptr<ReadableStateField<f_vector_t>> mtr_cmd_vec_fp;
   //! Temperature setpoint of the gyroscope heater.
   ReadableStateField<float> gyr_temp_eq_f;
   // We don't expect the following values to change very often, but we
   // send them on every control cycle anyway.
   //! Exponential filter coefficient for the reaction wheel angular momentum.
   WritableStateField<float> rwa_momentum_filter_f;
   //! Exponential filter coefficient for the reaction wheel ramp reads.
   WritableStateField<float> ramp_filter_f;
   //! Exponential filter coefficient for the sun sensor voltage reads.
   WritableStateField<float> ssa_voltage_filter_f;
   //! Exponential filter coefficient for the magnetometer readings.
   WritableStateField<float> mag_filter_f;
   //! Exponential filter coefficient for the gyroscope readings.
   WritableStateField<float> gyr_filter_f;
   //! Proportional gain for the gyroscope heater controller.
   WritableStateField<float> gyr_temp_kp_f;
   //! Integral gain for the gyroscope heater controller.
   WritableStateField<float> gyr_temp_ki_f;
   //! Derivative gain for the gyroscope heater controller.
   WritableStateField<float> gyr_temp_kd_f;
   //! Maximum-allowed commanded magnetic moment for the magnetorquers.
   WritableStateField<float> mtr_limit_f;
   //! Choice of magnetometer (magnetometer 1 or 2).
   WritableStateField<unsigned int> imu_mode_f;

   /**
    * @brief Fault flags.
    */
   //! True if the gyro heater has been unable to reach the temperature setpoint.
   ReadableStateField<bool> gyr_htr_failed;
};

#endif
