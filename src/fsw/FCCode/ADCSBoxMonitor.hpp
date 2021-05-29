#ifndef ADCS_BOX_MONITOR_HPP_
#define ADCS_BOX_MONITOR_HPP_

#include "Drivers/ADCS.hpp"
#include "TimedControlTask.hpp"
#include <common/Fault.hpp>

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
     * @param _adcs the input adcs system
     */
    ADCSBoxMonitor(StateFieldRegistry &registry, Devices::ADCS &_adcs);

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

    // Dummy vector serializer. This is used for values that still
    // need to be read/written from psim, but will not be downlinked
    // because their individual components are getting downlinked
    // instead.
    Serializer<lin::Vector3f> dummy_vec_sr;

    //! Speed reads off of the reaction wheels.
    Serializer<float> rwa_speed_rd_component_sr; // component serializer
    ReadableStateField<lin::Vector3f> rwa_speed_rd_f;
    ReadableStateField<float> rwa_speed_rd_x_f;
    ReadableStateField<float> rwa_speed_rd_y_f;
    ReadableStateField<float> rwa_speed_rd_z_f;

    //! Torque reads off of the reaction wheels.
    Serializer<float> rwa_torque_rd_component_sr;
    ReadableStateField<lin::Vector3f> rwa_torque_rd_f;
    ReadableStateField<float> rwa_torque_rd_x_f;
    ReadableStateField<float> rwa_torque_rd_y_f;
    ReadableStateField<float> rwa_torque_rd_z_f;

    Serializer<unsigned char> ssa_mode_rd;
    ReadableStateField<unsigned char> ssa_mode_f;

    //! Vector to the sun in the body frame.
    ReadableStateField<lin::Vector3f> ssa_vec_f;

    //! Raw voltages of the sun sensors.
    Serializer<float> ssa_voltage_sr;
    std::vector<ReadableStateField<float>> ssa_voltages_f;

    //! Magnetic field vector in the body frame of MAG1.
    Serializer<float> mag1_vec_component_sr;
    ReadableStateField<lin::Vector3f> mag1_vec_f;
    ReadableStateField<float> mag1_vec_x_f;
    ReadableStateField<float> mag1_vec_y_f;
    ReadableStateField<float> mag1_vec_z_f;

    //! Magnetic field vector in the body frame of MAG2.
    Serializer<float> mag2_vec_component_sr;
    ReadableStateField<lin::Vector3f> mag2_vec_f;
    ReadableStateField<float> mag2_vec_x_f;
    ReadableStateField<float> mag2_vec_y_f;
    ReadableStateField<float> mag2_vec_z_f;

    //! Angular rate vector in the body frame.
    Serializer<float> gyr_vec_component_sr;
    ReadableStateField<lin::Vector3f> gyr_vec_f;
    ReadableStateField<float> gyr_vec_x_f;
    ReadableStateField<float> gyr_vec_y_f;
    ReadableStateField<float> gyr_vec_z_f;

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
    ReadableStateField<bool> mag1_vec_flag;
    //! Is the magnetic field vector reading within the validity bounds?
    ReadableStateField<bool> mag2_vec_flag;
    //! Is the gyroscope rate reading within the validity bounds?
    ReadableStateField<bool> gyr_vec_flag;
    //! Is the gyroscope temperature reading within the validity bounds?W
    ReadableStateField<bool> gyr_temp_flag;

    //! vector for havt table, a 0/false means device disabled; 1/true is functional
    std::vector<ReadableStateField<bool>> havt_read_vector;
    Serializer<bool> havt_bool_sr;

    // RSF is named "adcs_monitor.functional", set to return of adcs_system.is_functional()
    ReadableStateField<bool> adcs_is_functional;
    //! Fault is thrown if is_functional() for ADCS itself doesn't work
    Fault adcs_functional_fault;

    // Faults are signaled based off of HAVT
    Fault wheel1_adc_fault;
    Fault wheel2_adc_fault;
    Fault wheel3_adc_fault;
    Fault wheel_pot_fault;
};

#endif
