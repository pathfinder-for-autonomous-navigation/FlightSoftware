#ifndef ADCS_BOX_CONTROLLER_HPP_
#define ADCS_BOX_CONTROLLER_HPP_

#include <ADCS.hpp>
#include <ControlTask.hpp>

class ADCSBoxController : public ControlTask<void> {
  public:
   ADCSBoxController(StateFieldRegistry& registry, Devices::ADCS& adcs);

   /**
    * @brief Writes outputs to the ADCS box from the values in the state fields
    * listed below.
    */
   void execute() override;

  protected:
   /** ADCS Driver. **/
   Devices::ADCS& adcs_system;

   /**
    * @brief Outputs to send to ADCS box.
    */
   ReadableStateField<bool> adcs_box_mode_f;
   ReadableStateField<f_vector_t> rwa_torque_vec_f;
   ReadableStateField<f_vector_t> rwa_speed_vec_f;
   ReadableStateField<f_vector_t> mtr_cmd_vec_f;
   ReadableStateField<float> gyr_temp_f;
   // We don't expect these values to change very often, but we
   // send them on every control cycle anyway.
   WritableStateField<float> rwa_momentum_filter_f;
   WritableStateField<float> ramp_filter_f;
   WritableStateField<float> ssa_voltage_filter_f;
   WritableStateField<float> mag_filter_f;
   WritableStateField<float> gyr_filter_f;
   WritableStateField<float> gyr_temp_kp_f;
   WritableStateField<float> gyr_temp_ki_f;
   WritableStateField<float> gyr_temp_kd_f;
   WritableStateField<float> mtr_limit_f;
   WritableStateField<unsigned int> imu_mode_f;
};

#endif
