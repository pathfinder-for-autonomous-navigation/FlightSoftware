#include "ADCSBoxMonitor.hpp"

#include <adcs_constants.hpp>
#include <string>
#include <sstream>

//#include "../../.pio/libdeps/native/CommonSoftware/include/adcs_constants.hpp"

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
    gyr_temp_f("adcs_monitor.gyr_temp", gyr_temp_sr)
    {

        std::stringstream ss;
        //actually create the state fields for ssa_voltages
        //only one serializer is needed

        char buffer [3];
        //return 0;

        for(unsigned int i = 0; i<20;i++){

            //sprintf!

            //ssa_voltages_f.emplace_back(ReadableStateField<float>("adcs_monitor.ssa_voltage"+std::to_string(i),ssa_voltage_sr));
            //ssa_voltages_f.emplace_back(ReadableStateField<float>("adcs_monitor.ssa_voltage"+ss.str(), ssa_voltage_sr));
            ssa_voltages_f.emplace_back(ReadableStateField<float>("adcs_monitor.ssa_voltage"+sprintf(buffer, "%u", i), ssa_voltage_sr));

                        


        }

        //actually add statefields to registry
        add_readable_field(rwa_speed_rd_f);
        add_readable_field(rwa_torque_rd_f);
        add_readable_field(ssa_mode_f);
        add_readable_field(ssa_vec_f);

        for(int i = 0; i<20; i++){
            add_readable_field(ssa_voltages_f[i]);
        }

        add_readable_field(mag_vec_f);
        add_readable_field(gyr_vec_f);
        add_readable_field(gyr_temp_f);
        
    }

void ADCSBoxMonitor::execute(){

    //create internal containers to read data
    f_vector_t rwa_speed_rd{};
    f_vector_t rwa_torque_rd{};
    unsigned char ssa_mode = 0;
    f_vector_t ssa_vector{};

    std::array<float, 20> ssa_voltages;
    ssa_voltages.fill(0);

    f_vector_t mag_vec{};
    f_vector_t gyr_vec{};
    float gyr_temp = 0.0;

    //ask the driver to fill in values
    adcs_system.get_rwa(&rwa_speed_rd,&rwa_torque_rd);
    adcs_system.get_ssa_mode(&ssa_mode);
    adcs_system.get_ssa_vector(&ssa_vector);
    adcs_system.get_ssa_voltage(&ssa_voltages);
    adcs_system.get_imu(&mag_vec, &gyr_vec, &gyr_temp);

    //set statefields from internal containers
    rwa_speed_rd_f.set(rwa_speed_rd);
    rwa_torque_rd_f.set(rwa_torque_rd);
    ssa_mode_f.set(ssa_mode);
    ssa_vec_f.set(ssa_vector);

    for(int i = 0; i<20; i++){
        ssa_voltages_f[i].set(ssa_voltages[i]);
    }

    mag_vec_f.set(mag_vec);
    gyr_vec_f.set(gyr_vec);
    gyr_temp_f.set(gyr_temp);

}