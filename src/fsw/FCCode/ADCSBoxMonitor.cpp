#include "ADCSBoxMonitor.hpp"

#include <adcs/constants.hpp>
#include <adcs/havt_devices.hpp>

ADCSBoxMonitor::ADCSBoxMonitor(StateFieldRegistry &registry, 
    unsigned int offset, Devices::ADCS &_adcs)
    : TimedControlTask<void>(registry, "adcs_monitor", offset),
    adcs_system(_adcs),
    rwa_speed_rd_sr(adcs::rwa::min_speed_read, adcs::rwa::max_speed_read, 16*3), //referenced from I2C_Interface.doc
    rwa_speed_rd_f("adcs_monitor.rwa_speed_rd", rwa_speed_rd_sr),
    rwa_torque_rd_sr(adcs::rwa::min_torque, adcs::rwa::max_torque, 16*3), //referenced from I2C_Interface.doc
    rwa_torque_rd_f("adcs_monitor.rwa_torque_rd", rwa_torque_rd_sr),
    ssa_mode_rd(0,2,2), //referenced from Interface.doc
    ssa_mode_f("adcs_monitor.ssa_mode", ssa_mode_rd),
    ssa_vec_sr(-1,1,16*3), //referenced from I2C_Interface.doc
    ssa_vec_f("adcs_monitor.ssa_vec", ssa_vec_sr),
    ssa_voltage_sr(adcs::ssa::min_voltage_rd, adcs::ssa::max_voltage_rd, 8),
    ssa_voltages_f(),
    mag1_vec_sr(adcs::imu::min_mag1_rd_mag, adcs::imu::max_mag1_rd_mag, 16*3), //referenced from I2C_Interface.doc
    mag1_vec_f("adcs_monitor.mag1_vec", mag1_vec_sr),
    mag2_vec_sr(adcs::imu::min_mag2_rd_mag, adcs::imu::max_mag2_rd_mag, 16*3), //referenced from I2C_Interface.doc
    mag2_vec_f("adcs_monitor.mag2_vec", mag2_vec_sr),
    gyr_vec_sr(adcs::imu::min_rd_omega, adcs::imu::max_rd_omega, 16*3), //referenced from I2C_Interface.doc
    gyr_vec_f("adcs_monitor.gyr_vec", gyr_vec_sr),
    gyr_temp_sr(adcs::ssa::min_voltage_rd, adcs::ssa::max_voltage_rd, 16), //referenced from I2C_Interface.doc
    gyr_temp_f("adcs_monitor.gyr_temp", gyr_temp_sr),
    flag_sr(),
    rwa_speed_rd_flag("adcs_monitor.speed_rd_flag", flag_sr),
    rwa_torque_rd_flag("adcs_monitor.torque_rd_flag", flag_sr),
    mag1_vec_flag("adcs_monitor.mag1_vec_flag", flag_sr),
    mag2_vec_flag("adcs_monitor.mag2_vec_flag", flag_sr),
    gyr_vec_flag("adcs_monitor.gyr_vec_flag", flag_sr),
    gyr_temp_flag("adcs_monitor.gyr_temp_flag", flag_sr),
    havt_bool_sr(),
    adcs_is_functional("adcs_monitor.functional", flag_sr),
    adcs_functional_fault("adcs_monitor.functional_fault", 1),
    wheel1_adc_fault("adcs_monitor.wheel1_fault", 1),
    wheel2_adc_fault("adcs_monitor.wheel2_fault", 1),
    wheel3_adc_fault("adcs_monitor.wheel3_fault", 1),
    wheel_pot_fault("adcs_monitor.wheel_pot_fault", 1)
    {
        // reserve memory
        ssa_voltages_f.reserve(adcs::ssa::num_sun_sensors);
        // fill vector of statefields for ssa
        char buffer[50];
        for(unsigned int i = 0; i<adcs::ssa::num_sun_sensors;i++){
            std::memset(buffer, 0, sizeof(buffer));
            sprintf(buffer,"adcs_monitor.ssa_voltage");
            sprintf(buffer + strlen(buffer), "%u", i);
            ssa_voltages_f.emplace_back(buffer, ssa_voltage_sr);
        }

        havt_read_vector.reserve(adcs::havt::Index::_LENGTH);
        // fill vector of statefields for havt
        for (unsigned int idx = adcs::havt::Index::IMU_GYR; idx < adcs::havt::Index::_LENGTH; idx++ )
        {
            std::memset(buffer, 0, sizeof(buffer));
            sprintf(buffer,"adcs_monitor.havt_device");
            sprintf(buffer + strlen(buffer), "%u", idx);
            havt_read_vector.emplace_back(buffer, havt_bool_sr);
        }
        
        // add device availabilty to registry, and initialize value to 0
        for(unsigned int idx = adcs::havt::Index::IMU_GYR; idx < adcs::havt::Index::_LENGTH; idx++ )
        {
            add_readable_field(havt_read_vector[idx]);
            havt_read_vector[idx].set(false);
        }

        //actually add statefields to registry
        add_readable_field(rwa_speed_rd_f);
        add_readable_field(rwa_torque_rd_f);
        add_readable_field(ssa_mode_f);
        add_readable_field(ssa_vec_f);

        for(unsigned int i = 0; i<adcs::ssa::num_sun_sensors; i++){
            add_readable_field(ssa_voltages_f[i]);
        }

        add_readable_field(mag1_vec_f);
        add_readable_field(mag2_vec_f);
        add_readable_field(gyr_vec_f);
        add_readable_field(gyr_temp_f);

        //add flag state fields
        add_readable_field(rwa_speed_rd_flag);
        add_readable_field(rwa_torque_rd_flag);
        add_readable_field(mag1_vec_flag);
        add_readable_field(mag2_vec_flag);
        add_readable_field(gyr_vec_flag);
        add_readable_field(gyr_temp_flag);

        add_readable_field(adcs_is_functional);
        // add faults to registry
        add_fault(adcs_functional_fault);
        add_fault(wheel1_adc_fault);
        add_fault(wheel2_adc_fault);
        add_fault(wheel3_adc_fault);
        add_fault(wheel_pot_fault);
    }

bool exceed_bounds(const std::array<float, 3>& input, const float min, const float max){
    for(int i = 0; i<3; i++){
        if(input[i] < min || input[i] > max){
            return true;
        }
    }
    return false;
}

bool exceed_bounds(const float input, const float min, const float max){
    if(input < min || input > max)
        return true;
    return false;
}

// TODO: WOULD BE A NICE FEATURE IN LIN.hpp
template<size_t N>
static lin::Vector<float, N> to_linvector(const std::array<float, N>& src) {
    lin::Vector<float, N> src_cpy;
    for(unsigned int i = 0; i < N; i++) src_cpy(i) = src[i];
    return src_cpy;
}

void ADCSBoxMonitor::execute(){

    //define nan
    const float nan = std::numeric_limits<float>::quiet_NaN();

    //create internal containers to read data
    f_vector_t rwa_speed_rd;
    f_vector_t rwa_torque_rd;
    unsigned char ssa_mode = 0;
    f_vector_t ssa_vec;

    std::array<float, adcs::ssa::num_sun_sensors> ssa_voltages;
    ssa_voltages.fill(0);

    f_vector_t mag1_vec;
    f_vector_t mag2_vec;
    f_vector_t gyr_vec;
    float gyr_temp = 0.0;
    
    // Determine whether a fault has occurred. Evaluating faults before reading from the adcs
    // system allows us to test fault responses in ptest.
    adcs_functional_fault.evaluate(!adcs_is_functional.get());

    //ask the driver to fill in values
    adcs_is_functional.set(adcs_system.i2c_ping());

    adcs_system.get_rwa(&rwa_speed_rd,&rwa_torque_rd);
    adcs_system.get_ssa_voltage(&ssa_voltages);
    adcs_system.get_imu(&mag1_vec, &mag2_vec, &gyr_vec, &gyr_temp);

    //only update the ssa_vector if and only if the mode was COMPLETE
    adcs_system.get_ssa_mode(&ssa_mode);
    if(ssa_mode == adcs::SSAMode::SSA_COMPLETE){
        adcs_system.get_ssa_vector(&ssa_vec);
        ssa_vec_f.set(to_linvector(ssa_vec));
    }
    else{
        ssa_vec_f.set({nan,nan,nan});
    }
    
    //set statefields from internal containers
    rwa_speed_rd_f.set(to_linvector(rwa_speed_rd));
    rwa_torque_rd_f.set(to_linvector(rwa_torque_rd));
    ssa_mode_f.set(ssa_mode);

    for(unsigned int i = 0; i<adcs::ssa::num_sun_sensors; i++){
        ssa_voltages_f[i].set(ssa_voltages[i]);
    }
    
    // Determine whether a fault has occurred. Evaluating faults before reading from the adcs
    // system allows us to test fault responses in ptest.
    wheel1_adc_fault.evaluate(havt_read_vector[adcs::havt::Index::RWA_ADC1].get() == false);
    wheel2_adc_fault.evaluate(havt_read_vector[adcs::havt::Index::RWA_ADC2].get() == false);
    wheel3_adc_fault.evaluate(havt_read_vector[adcs::havt::Index::RWA_ADC3].get() == false);
    wheel_pot_fault.evaluate(havt_read_vector[adcs::havt::Index::RWA_POT].get() == false);

    // set vector of device availability
    std::bitset<adcs::havt::max_devices> havt_read(0);
    adcs_system.get_havt(&havt_read);
    for(unsigned int idx = adcs::havt::Index::IMU_GYR; idx < adcs::havt::Index::_LENGTH; idx++ )
    {
        havt_read_vector[idx].set(havt_read.test(idx));
    }

    mag1_vec_f.set(to_linvector(mag1_vec));
    mag2_vec_f.set(to_linvector(mag2_vec));
    gyr_vec_f.set(to_linvector(gyr_vec));
    gyr_temp_f.set(gyr_temp);

    //flags default to false, meaning there are no issues
    rwa_speed_rd_flag.set(false);
    rwa_torque_rd_flag.set(false);
    mag1_vec_flag.set(false);
    mag2_vec_flag.set(false);
    gyr_vec_flag.set(false);
    gyr_temp_flag.set(false);

    //TODO: UPDATE; THESE ARE PLACE HOLDER FLAG BOUNDS
    //They all have bounds of min to max -1 to force a flag for testing purposes
    //Eventually change to proper bounds
    if(exceed_bounds(rwa_speed_rd, adcs::rwa::min_speed_read, adcs::rwa::max_speed_read - 1))
        rwa_speed_rd_flag.set(true);
    if(exceed_bounds(rwa_torque_rd, adcs::rwa::min_torque, adcs::rwa::max_torque - 1))
        rwa_torque_rd_flag.set(true);
    if(exceed_bounds(mag1_vec, adcs::imu::min_mag1_rd_mag, adcs::imu::max_mag1_rd_mag - 1))
        mag1_vec_flag.set(true);
    if(exceed_bounds(mag2_vec, adcs::imu::min_mag1_rd_mag, adcs::imu::max_mag1_rd_mag - 1))
        mag2_vec_flag.set(true);
    if(exceed_bounds(gyr_vec, adcs::imu::min_rd_omega, adcs::imu::max_rd_omega - 1))
        gyr_vec_flag.set(true);
    if(exceed_bounds(gyr_temp, adcs::imu::min_rd_temp, adcs::imu::max_rd_temp - 1))
        gyr_temp_flag.set(true);
}