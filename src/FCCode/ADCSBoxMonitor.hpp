#ifndef ADCS_BOX_MONITOR_HPP_
#define ADCS_BOX_MONITOR_HPP_

#include <ADCS.hpp>
#include <ControlTask.hpp>

class ADCSBoxMonitor : public ControlTask<void> {
  public:
   ADCSBoxMonitor(StateFieldRegistry& registry, Devices::ADCS& adcs);

   /**
    * @brief Gets inputs from the ADCS box and dumps them into the state
    * fields listed below.
    */
   void execute() override;

  protected:
   /** ADCS Driver. **/
   Devices::ADCS& adcs_system;

   /**
    * @brief Inputs to get from ADCS box.
    */
   ReadableStateField<f_vector_t> rwa_rd_f;
   ReadableStateField<f_vector_t> ssa_vec_f;
   ReadableStateField<bool> ssa_vec_valid_f;
   ReadableStateField<f_vector_t> mag_vec_f;
   ReadableStateField<f_vector_t> gyr_vec_f;
   ReadableStateField<float> gyr_temp_f;
};

#endif
