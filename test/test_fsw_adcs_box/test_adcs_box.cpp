#include "../StateFieldRegistryMock.hpp"

#include "../../src/FCCode/ADCSBoxMonitor.hpp"
//#include <ADCSBoxMonitor.hpp>

#include <unity.h>

class TestFixture {
    public:
        StateFieldRegistryMock registry;

        // pointers to output statefields for easy access
        ReadableStateField<f_quat_t>* q_body_eci_fp;
        ReadableStateField<f_vector_t>* w_body_fp;

        // adcs_system(_adcs),
        // rwa_momentum_rd_sr(rwa::min_momentum, rwa::max_momentum, 16*3), //referenced from I2C_Interface.doc
        // rwa_momentum_rd_f("adcs_box.rwa_speed_rd", rwa_momentum_rd_sr),
        // rwa_torque_rd_sr(rwa::min_torque, rwa::max_torque, 16*3), //referenced from I2C_Interface.doc
        // rwa_torque_rd_f("adcs_box.torque_rd", rwa_torque_rd_sr),
        // ssa_mode_rd(0,2,2), //referenced from Interface.doc
        // ssa_mode_f("adcs_box.ssa_mode", ssa_mode_rd),
        // ssa_vec_rd_sr(-1,1,16*3), //referenced from I2C_Interface.doc
        // ssa_vec_rd_f("adcs_box.ssa_vec_rd", ssa_vec_rd_sr),
        // ssa_voltage_sr(ssa::min_voltage_rd, ssa::max_voltage_rd, 8),
        // ssa_voltages_f(),
        // mag_vec_sr(imu::min_rd_mag, imu::max_rd_mag, 16*3), //referenced from I2C_Interface.doc
        // mag_vec_f("adcs_box.mag_vec", mag_vec_sr),
        // gyr_vec_sr(imu::min_rd_omega, imu::max_rd_omega, 16*3), //referenced from I2C_Interface.doc
        // gyr_vec_f("adcx_box.gyr_vec", gyr_vec_sr),
        // gyr_temp_sr(ssa::min_voltage_rd, ssa::max_voltage_rd, 16), //referenced from I2C_Interface.doc
        // gyr_temp_f("adcs_box.gyr_temp", gyr_temp_sr)

        std::unique_ptr<ADCSBoxMonitor> adcs_box;

        // Create a TestFixture instance of AttitudeEstimator with pointers to statefields
        TestFixture() : registry(){

                //create input statefields
                piksi_time_fp = registry.create_readable_field<gps_time_t>("piksi.time");
                pos_vec_ecef_fp = registry.create_readable_vector_field<double>("piksi.pos",0.0L,1000000.0L,64*3);
                ssa_vec_rd_fp = registry.create_readable_vector_field<float>("adcs_box.sun_vec",-1.0,1.0,32*3),
                mag_vec_fp = registry.create_readable_vector_field<float>("adcs_box.mag_vec",-16e-4,16e4,32*3),

                attitude_estimator = std::make_unique<AttitudeEstimator>(registry, 0);  

                // initialize pointers to statefields
                q_body_eci_fp = registry.find_readable_field_t<f_quat_t>("attitude_estimator.q_body_eci");
                w_body_fp = registry.find_readable_field_t<f_vector_t>("attitude_estimator.w_body");

                assert(q_body_eci_fp);
                assert(w_body_fp);
        
        }
};

void test_task_initialization()
{
        TestFixture tf;
}

void test_execute(){

}

int test_control_task()
{
        UNITY_BEGIN();
        RUN_TEST(test_task_initialization);
        RUN_TEST(test_execute);
        return UNITY_END();
}

#ifdef DESKTOP
int main()
{
        return test_control_task();
}
#else
#include <Arduino.h>
void setup()
{
        delay(2000);
        Serial.begin(9600);
        test_control_task();
}

void loop() {}
#endif