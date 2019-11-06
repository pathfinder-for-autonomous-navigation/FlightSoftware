#ifndef ADCS_BOX_MONITOR_HPP_
#define ADCS_BOX_MONITOR_HPP_

#include <ADCS.hpp>
#include <ControlTask.hpp>

/**
  * @brief Gets inputs from the ADCS box and dumps them into the state
  * fields listed below.
  */
class ADCSBoxMonitor : public ControlTask<void> {
  public:
   /**
    * @brief Construct a new ADCSBoxMonitor
    * 
    * @param registry 
    * @param adcs 
    */
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
   //! Speed reads off of the reaction wheels.
   ReadableStateField<f_vector_t> rwa_rd_f;
   //! Vector to the sun in the body frame.
   ReadableStateField<f_vector_t> ssa_vec_f;
   //! Confidence in sun vector estimation.
   ReadableStateField<bool> ssa_vec_valid_f;
   //! Magnetic field vector in the body frame.
   ReadableStateField<f_vector_t> mag_vec_f;
   //! Angular rate vector in the body frame.
   ReadableStateField<f_vector_t> gyr_vec_f;
   //! Temperature near the gyroscope.
   ReadableStateField<float> gyr_temp_f;
};

#endif
