#include "ADCSBoxMonitor.hpp"

ADCSBoxMonitor::ADCSBoxMonitor(StateFieldRegistry &registry, 
    unsigned int offset, Devices::ADCS &_adcs)
    : TimedControlTask<void>(registry,offset),
    adcs_system(_adcs),
    rwa_speed_rd_sr(-1,1,32*3),
    rwa_speed_rd_f("adcs_box.rwa_speed_rd", rwa_speed_rd_sr),
    rwa_torque_rd_sr(-1,1,32*3),
    rwa_torque_rd_f("adcs_box.torque_rd", rwa_torque_rd_sr),
    ssa_vec_rd_sr(-1,1,32*3),
    ssa_vec_rd_f("adcs_box.ssa_vec_rd", ssa_vec_rd_sr),
    //ssa_voltages_sr(),
    //std::vector<Serailizer<float>, num_sun_sensors> ssa_voltage_sr(),
    //ssa_voltages_sr(std::vector<float, num_sun_sensors>);
    //ssa_voltages_f("adcs_box.ssa_voltages", ssa_voltages_sr),
    mag_vec_sr(-1,1,32*3),
    mag_vec_f("adcs_box.mag_vec", mag_vec_sr),
    gyr_vec_sr(-1,1,32*3),
    gyr_vec_f("adcx_box.gyr_vec", gyr_vec_sr),
    gyr_temp_sr(0,100,10),
    gyr_temp_f("adcs_box.gyr_temp", gyr_temp_sr)
    {
    // //! Speed reads off of the reaction wheels.
    // ReadableStateField<f_vector_t> rwa_speed_rd_f;
    // Serializer<f_vector_t> rwa_speed_rd_sr;

    // //! Torque reads off of the reaction wheels.
    // ReadableStateField<f_vector_t> rwa_torque_rd_f;
    // Serializer<f_vector_t> rwa_torque_rd_sr;

    // //! Vector to the sun in the body frame.
    // ReadableStateField<f_vector_t> ssa_vec_rd_f;
    // Serializer<f_vector_t> ssa_vec_rd_sr;

    // //! Raw voltages of the sun sensors.
    // std::vector<ReadableStateField<float>> ssa_voltages_f;
    // std::vector<Serializer<float>> ssa_voltages_sr;

    // //! Magnetic field vector in the body frame.
    // ReadableStateField<f_vector_t> mag_vec_f;
    // Serializer<f_vector_t> mag_vec_sr;

    // //! Angular rate vector in the body frame.
    // ReadableStateField<f_vector_t> gyr_vec_f;
    // Serializer<f_vector_t> gyr_vec_sr;

    // //! Temperature near the gyroscope.
    // ReadableStateField<float> gyr_temp_f;
    // Serializer<float> gyr_temp_sr;

        
    }

void ADCSBoxMonitor::execute(){
    
}