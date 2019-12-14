#include "ADCSBoxMonitor.hpp"

#include <adcs_constants.hpp>

ADCSBoxMonitor::ADCSBoxMonitor(StateFieldRegistry &registry, 
    unsigned int offset, Devices::ADCS &_adcs)
    : TimedControlTask<void>(registry,offset),
    adcs_system(_adcs),
    rwa_speed_rd_sr(rwa::min_momentum, rwa::max_momentum, 16*3), //referenced from I2C_Interface.doc
    rwa_speed_rd_f("adcs_monitor.rwa_speed_rd", rwa_speed_rd_sr),
    rwa_torque_rd_sr(rwa::min_torque, rwa::max_torque, 16*3), //referenced from I2C_Interface.doc
    rwa_torque_rd_f("adcs_monitor.rwa_torque_rd", rwa_torque_rd_sr),
    ssa_mode_rd(0,2,2), //referenced from Interface.doc
    ssa_mode_f("adcs_monitor.ssa_mode", ssa_mode_rd),
    ssa_vec_sr(-1,1,16*3), //referenced from I2C_Interface.doc
    ssa_vec_f("adcs_monitor.ssa_vec", ssa_vec_sr),
    ssa_voltage_sr(ssa::min_voltage_rd, ssa::max_voltage_rd, 8),
    ssa_voltages_f(),
    mag_vec_sr(imu::min_rd_mag, imu::max_rd_mag, 16*3), //referenced from I2C_Interface.doc
    mag_vec_f("adcs_monitor.mag_vec", mag_vec_sr),
    gyr_vec_sr(imu::min_rd_omega, imu::max_rd_omega, 16*3), //referenced from I2C_Interface.doc
    gyr_vec_f("adcs_monitor.gyr_vec", gyr_vec_sr),
    gyr_temp_sr(ssa::min_voltage_rd, ssa::max_voltage_rd, 16), //referenced from I2C_Interface.doc
    gyr_temp_f("adcs_monitor.gyr_temp", gyr_temp_sr),
    flag_sr(),
    rwa_speed_rd_flag("adcs_monitor.speed_rd_flag", flag_sr),
    rwa_torque_rd_flag("adcs_monitor.torque_rd_flag", flag_sr),
    ssa_vec_flag("adcs_monitor.ssa_vec_flag", flag_sr),
    mag_vec_flag("adcs_monitor.mag_vec_flag", flag_sr),
    gyr_vec_flag("adcs_monitor.gyr_vec_flag", flag_sr),
    gyr_temp_flag("adcs_monitor.gyr_temp_flag", flag_sr)
    {
        //fill vector of statefields
        //char buffer [3];
        char buffer[50];
        for(unsigned int i = 0; i<num_sun_sensors;i++){
            std::memset(buffer, 0, sizeof(buffer));
            sprintf(buffer,"adcs_monitor.ssa_voltage");
            sprintf(buffer + strlen(buffer), "%u", i);
            ssa_voltages_f.push_back(ReadableStateField<float>(buffer, ssa_voltage_sr));
        }

        //actually add statefields to registry
        add_readable_field(rwa_speed_rd_f);
        add_readable_field(rwa_torque_rd_f);
        add_readable_field(ssa_mode_f);
        add_readable_field(ssa_vec_f);

        for(unsigned int i = 0; i<num_sun_sensors; i++){
            add_readable_field(ssa_voltages_f[i]);
        }

        add_readable_field(mag_vec_f);
        add_readable_field(gyr_vec_f);
        add_readable_field(gyr_temp_f);

        //add flag state fields
        add_readable_field(rwa_speed_rd_flag);
        add_readable_field(rwa_torque_rd_flag);
        add_readable_field(ssa_vec_flag);
        add_readable_field(mag_vec_flag);
        add_readable_field(gyr_vec_flag);
        add_readable_field(gyr_temp_flag);
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

void ADCSBoxMonitor::execute(){

    //create internal containers to read data
    f_vector_t rwa_speed_rd;
    f_vector_t rwa_torque_rd;
    unsigned char ssa_mode = 0;
    f_vector_t ssa_vec;

    std::array<float, 20> ssa_voltages;
    ssa_voltages.fill(0);

    f_vector_t mag_vec;
    f_vector_t gyr_vec;
    float gyr_temp = 0.0;

    //ask the driver to fill in values
    adcs_system.get_rwa(&rwa_speed_rd,&rwa_torque_rd);
    adcs_system.get_ssa_voltage(&ssa_voltages);
    adcs_system.get_imu(&mag_vec, &gyr_vec, &gyr_temp);

    //only update the ssa_vector if and only if the mode was COMPLETE
    adcs_system.get_ssa_mode(&ssa_mode);
    if(ssa_mode == SSAMode::SSA_COMPLETE){
        adcs_system.get_ssa_vector(&ssa_vec);
    }

    //set statefields from internal containers
    rwa_speed_rd_f.set(rwa_speed_rd);
    rwa_torque_rd_f.set(rwa_torque_rd);
    ssa_mode_f.set(ssa_mode);
    ssa_vec_f.set(ssa_vec);

    for(int i = 0; i<20; i++){
        ssa_voltages_f[i].set(ssa_voltages[i]);
    }

    mag_vec_f.set(mag_vec);
    gyr_vec_f.set(gyr_vec);
    gyr_temp_f.set(gyr_temp);

    //flags default to false, meaning there are no issues
    rwa_speed_rd_flag.set(false);
    rwa_torque_rd_flag.set(false);
    ssa_vec_flag.set(false);
    mag_vec_flag.set(false);
    gyr_vec_flag.set(false);
    gyr_temp_flag.set(false);

    //TODO: UPDATE; THESE ARE PLACE HOLDER FLAG BOUNDS
    //They all have bounds of min to max -1 to force a flag for testing purposes
    //Eventually change to proper bounds
    if(exceed_bounds(rwa_speed_rd, rwa::min_speed_read, rwa::max_speed_read - 1))
        rwa_speed_rd_flag.set(true);
    if(exceed_bounds(rwa_torque_rd, rwa::min_torque, rwa::max_torque - 1))
        rwa_torque_rd_flag.set(true);
    if(exceed_bounds(ssa_vec, -1, 1 - 1))
        ssa_vec_flag.set(true);
    if(exceed_bounds(mag_vec, imu::min_rd_mag, imu::max_rd_mag - 1))
        mag_vec_flag.set(true);
    if(exceed_bounds(gyr_vec, imu::min_rd_omega, imu::max_rd_omega - 1))
        gyr_vec_flag.set(true);
    if(exceed_bounds(gyr_temp, imu::min_rd_temp, imu::max_rd_temp - 1))
        gyr_temp_flag.set(true);
}