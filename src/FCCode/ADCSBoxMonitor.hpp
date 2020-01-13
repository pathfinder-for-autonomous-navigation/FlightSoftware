#ifndef ADCS_BOX_MONITOR_HPP_
#define ADCS_BOX_MONITOR_HPP_

#include <ADCS.hpp>
#include "TimedControlTask.hpp"

/**
* @brief Gets inputs from the ADCS box and dumps them into the state
* fields listed below.
*/
class ADCSBoxMonitor : public TimedControlTask<void>
{
public:
    /**
     * @brief Construct a new ADCSBoxMonitor control task
     * 
     * @param registry input StateField registry
     * @param offset control task offset
     * @param _adcs the input adcs system
     */
    ADCSBoxMonitor(StateFieldRegistry &registry, unsigned int offset, Devices::ADCS &_adcs);

    /** ADCS Driver. **/
    Devices::ADCS& adcs_system;

    /**
    * @brief Gets inputs from the ADCS box and dumps them into the state
    * fields listed below.
    */
    void execute() override;

protected:
    /**
    * @brief Inputs to get from ADCS box.
    */

    //! Speed reads off of the reaction wheels.
    Serializer<f_vector_t> rwa_speed_rd_sr;
    ReadableStateField<f_vector_t> rwa_speed_rd_f;

    //! Torque reads off of the reaction wheels.
    Serializer<f_vector_t> rwa_torque_rd_sr;
    ReadableStateField<f_vector_t> rwa_torque_rd_f;

    Serializer<int> ssa_mode_rd;
    ReadableStateField<int> ssa_mode_f;

    //! Vector to the sun in the body frame.
    Serializer<f_vector_t> ssa_vec_sr;
    ReadableStateField<f_vector_t> ssa_vec_f;

    //! Raw voltages of the sun sensors.
    Serializer<float> ssa_voltage_sr;
    std::vector<ReadableStateField<float>> ssa_voltages_f;

    //! Magnetic field vector in the body frame.
    Serializer<f_vector_t> mag_vec_sr;
    ReadableStateField<f_vector_t> mag_vec_f;

    //! Angular rate vector in the body frame.
    Serializer<f_vector_t> gyr_vec_sr;
    ReadableStateField<f_vector_t> gyr_vec_f;

    //! Temperature near the gyroscope.
    Serializer<float> gyr_temp_sr;
    ReadableStateField<float> gyr_temp_f;


    /**
    * @brief Bound flags.
    * If the flag is true, that means it is outside the validity bounds.
    */
    Serializer<bool> flag_sr;
    //! Is the reaction wheel speed read outside the validity bounds?
    ReadableStateField<bool> rwa_speed_rd_flag;
    //! Is the reaction wheel torque read outside the validity bounds?
    ReadableStateField<bool> rwa_torque_rd_flag;
    //! Is the magnetic field vector reading within the validity bounds?
    ReadableStateField<bool> mag_vec_flag;
    //! Is the gyroscope rate reading within the validity bounds?
    ReadableStateField<bool> gyr_vec_flag;
    //! Is the gyroscope temperature reading within the validity bounds?W
    ReadableStateField<bool> gyr_temp_flag;
};

#endif
