#include "ADCSBoxController.hpp"
#include "adcs_state_t.enum"

#include <adcs_constants.hpp>

ADCSBoxController::ADCSBoxController(StateFieldRegistry &registry, 
    unsigned int offset, Devices::ADCS &_adcs)
    : TimedControlTask<void>(registry, "adcs_controller", offset),
    adcs_system(_adcs)
    {

        //find command statefields
        adcs_state_fp = find_writable_field<unsigned char>("adcs.state", __FILE__, __LINE__);

        const WritableStateField<unsigned char>* mtr_mode_fp;
        const WritableStateField<f_vector_t>* mtr_cmd_fp;
        const WritableStateField<float>* mtr_limit_fp;

        //perhaps change box monitor to a writeable statefield
        const WritableStateField<unsigned char>* ssa_mode_fp;

        //consider changing
        ssa_mode_fp = find_writable_field<unsigned char>("adcs_cmd.ssa_mode", __FILE__, __LINE__);
        ssa_voltage_filter_fp = find_writable_field<float>("adcs_cmd.ssa_voltage_filter", __FILE__, __LINE__);

        imu_mode_fp = find_writable_field<unsigned char>("adcs_cmd.imu_mode", __FILE__, __LINE__);
        imu_mag_filter_fp = find_writable_field<float>("adcs_cmd.imu_mag_filter", __FILE__, __LINE__);
        imu_gyr_filter_fp = find_writable_field<float>("adcs_cmd.imu_gyr_filter", __FILE__, __LINE__);
        imu_gyr_temp_filter_fp = find_writable_field<float>("adcs_cmd.imu_gyr_temp_filter", __FILE__, __LINE__);
        imu_gyr_temp_kp = find_writable_field<float>("adcs_cmd.imu_temp_kp", __FILE__, __LINE__);
        imu_gyr_temp_ki = find_writable_field<float>("adcs_cmd.imu_temp_ki", __FILE__, __LINE__);
        imu_gyr_temp_kd = find_writable_field<float>("adcs_cmd.imu_temp_kd", __FILE__, __LINE__);
        imu_gyr_temp_desired = find_writable_field<float>("adcs_cmd.imu_gyr_temp_desired", __FILE__, __LINE__);
    }

void ADCSBoxController::execute(){

    //define nan
    const float nan = std::numeric_limits<float>::quiet_NaN();

    if(adcs_state_fp->get() != adcs_state_t::startup){
        //enable the adcs_box
    }
    else
        //disable adcs box

    //update ssa calc mode

}