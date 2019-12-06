#ifndef ADCS_BOX_MONITOR_HPP_
#define ADCS_BOX_MONITOR_HPP_

#include <ADCS.hpp>
#include <TimedControlTask.hpp>

/**
* @brief Gets inputs from the ADCS box and dumps them into the state
* fields listed below.
*/
class ADCSBoxMonitor : public TimedControlTask<void>
{
public:
    /**
    * @brief Construct a new ADCSBoxMonitor
    * 
    * @param registry 
    * @param _adcs 
    */
    ADCSBoxMonitor(StateFieldRegistry &registry, unsigned int offset, Devices::ADCS &_adcs);

    /**
    * @brief Gets inputs from the ADCS box and dumps them into the state
    * fields listed below.
    */
    void execute() override;

    static constexpr unsigned int num_sun_sensors = 20;

protected:
    /** ADCS Driver. **/
    const Devices::ADCS& adcs_system;

    /**
    * @brief Inputs to get from ADCS box.
    */

    //! Speed reads off of the reaction wheels.
    ReadableStateField<f_vector_t> rwa_speed_rd_f;
    Serializer<f_vector_t> rwa_speed_rd_sr;

    //! Torque reads off of the reaction wheels.
    ReadableStateField<f_vector_t> rwa_torque_rd_f;
    Serializer<f_vector_t> rwa_torque_rd_sr;

    //! Vector to the sun in the body frame.
    ReadableStateField<f_vector_t> ssa_vec_rd_f;
    Serializer<f_vector_t> ssa_vec_rd_sr;

    //! Raw voltages of the sun sensors.
    // std::vector<ReadableStateField<float>> ssa_voltages_f;
    // std::vector<Serializer<float>> ssa_voltages_sr;

    //! Magnetic field vector in the body frame.
    ReadableStateField<f_vector_t> mag_vec_f;
    Serializer<f_vector_t> mag_vec_sr;

    //! Angular rate vector in the body frame.
    ReadableStateField<f_vector_t> gyr_vec_f;
    Serializer<f_vector_t> gyr_vec_sr;

    //! Temperature near the gyroscope.
    ReadableStateField<float> gyr_temp_f;
    Serializer<float> gyr_temp_sr;


    /**
        * @brief Fault flags.
        */
    // //! Is the reaction wheel speed read within the validity bounds?
    // ReadableStateField<bool> rwa_speed_rd_valid_f;
    // //! Is the reaction wheel torque read within the validity bounds?
    // ReadableStateField<bool> rwa_torque_rd_valid_f;
    // //! Confidence in sun vector estimation (reported by ADCS directly.)
    // ReadableStateField<bool> ssa_vec_rd_valid_f;
    // //! Is the magnetic field vector reading within the validity bounds?
    // ReadableStateField<bool> mag_vec_valid_f;
    // //! Is the gyroscope rate reading within bounds?
    // ReadableStateField<bool> gyr_vec_valid_f;
    // //! Is the gyroscope temperature reading within the validity bounds?
    // ReadableStateField<float> gyr_temp_valid_f;
};

#endif
