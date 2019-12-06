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
    ssa_voltage_sr(-1,1,32),
    //std::vector<Serailizer<float>, num_sun_sensors> ssa_voltage_sr(),
    //ssa_voltages_sr(Serializer<float> temp(-1,1,32)),
    //ssa_voltages_f("adcs_box.ssa_voltages", ssa_voltages_sr),
    ssa_voltages_f(),
    mag_vec_sr(-1,1,32*3),
    mag_vec_f("adcs_box.mag_vec", mag_vec_sr),
    gyr_vec_sr(-1,1,32*3),
    gyr_vec_f("adcx_box.gyr_vec", gyr_vec_sr),
    gyr_temp_sr(0,100,32),
    gyr_temp_f("adcs_box.gyr_temp", gyr_temp_sr)
    {
        //actually create the state fields for ssa_voltages
        //only one serializer is needed
        for(int i = 0; i<20;i++)
            ssa_voltages_f.emplace_back(ReadableStateField<float>("temp",ssa_voltage_sr));
        
    }

void ADCSBoxMonitor::execute(){
    
}