#include "ADCSBoxMonitor.hpp"
//#include <adcs_constants.hpp>
//#include "../../.pio/build/native/CommonSoftware/include/adcs_constants.hpp"
#include "../../.pio/libdeps/native/CommonSoftware/include/adcs_constants.hpp"

ADCSBoxMonitor::ADCSBoxMonitor(StateFieldRegistry &registry, 
    unsigned int offset, Devices::ADCS &_adcs)
    : TimedControlTask<void>(registry,offset),
    adcs_system(_adcs),
    rwa_momentum_rd_sr(rwa::min_momentum, rwa::max_momentum, 16*3), //referenced from I2C_Interface.doc
    rwa_momentum_rd_f("adcs_box.rwa_speed_rd", rwa_momentum_rd_sr),
    rwa_torque_rd_sr(rwa::min_torque, rwa::max_torque, 16*3), //referenced from I2C_Interface.doc
    rwa_torque_rd_f("adcs_box.torque_rd", rwa_torque_rd_sr),
    ssa_mode_rd(0,2,2), //referenced from Interface.doc
    ssa_mode_f("adcs_box.ssa_mode", ssa_mode_rd),
    ssa_vec_rd_sr(-1,1,16*3), //referenced from I2C_Interface.doc
    ssa_vec_rd_f("adcs_box.ssa_vec_rd", ssa_vec_rd_sr),
    ssa_voltage_sr(ssa::min_voltage_rd, ssa::max_voltage_rd, 8),
    ssa_voltages_f(),
    mag_vec_sr(imu::min_rd_mag, imu::max_rd_mag, 16*3), //referenced from I2C_Interface.doc
    mag_vec_f("adcs_box.mag_vec", mag_vec_sr),
    gyr_vec_sr(imu::min_rd_omega, imu::max_rd_omega, 16*3), //referenced from I2C_Interface.doc
    gyr_vec_f("adcx_box.gyr_vec", gyr_vec_sr),
    gyr_temp_sr(ssa::min_voltage_rd, ssa::max_voltage_rd, 16), //referenced from I2C_Interface.doc
    gyr_temp_f("adcs_box.gyr_temp", gyr_temp_sr)
    {
        //actually create the state fields for ssa_voltages
        //only one serializer is needed
        for(int i = 0; i<20;i++)
            ssa_voltages_f.emplace_back(ReadableStateField<float>("temp",ssa_voltage_sr));
        
    }

void ADCSBoxMonitor::execute(){
    
}