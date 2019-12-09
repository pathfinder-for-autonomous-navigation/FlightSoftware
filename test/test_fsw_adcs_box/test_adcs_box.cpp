#include "../StateFieldRegistryMock.hpp"

#include <ADCS.hpp>
#include "../../src/FCCode/ADCSBoxMonitor.hpp"
//#include <ADCSBoxMonitor.hpp>
#include "adcs_constants.hpp"

#include <unity.h>
#include <string>

class TestFixture {
    public:
        StateFieldRegistryMock registry;

        // pointers to output statefields for easy access
        ReadableStateField<f_vector_t>* rwa_speed_rd_fp;
        ReadableStateField<f_vector_t>* rwa_torque_rd_fp;
        ReadableStateField<int>* ssa_mode_fp;
        ReadableStateField<f_vector_t>* ssa_vec_fp;
        std::vector<ReadableStateField<float>*> ssa_voltages_fp{};
        ReadableStateField<f_vector_t>* mag_vec_fp;
        ReadableStateField<f_vector_t>* gyr_vec_fp;
        ReadableStateField<float>* gyr_temp_fp;
        
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

        Devices::ADCS adcs;// = adcs("adcs", 0);

        unsigned char addr = 0;
        // Create a TestFixture instance of AttitudeEstimator with pointers to statefields
        TestFixture() : registry(), adcs("adcs", addr){

            adcs_box = std::make_unique<ADCSBoxMonitor>(registry, 0, adcs);  

            // initialize pointers to statefields
            rwa_speed_rd_fp = registry.find_readable_field_t<f_vector_t>("adcs_monitor.rwa_speed_rd");
            rwa_torque_rd_fp = registry.find_readable_field_t<f_vector_t>("adcs_monitor.rwa_torque_rd");
            ssa_mode_fp = registry.find_readable_field_t<int>("adcs_monitor.ssa_mode");
            ssa_vec_fp = registry.find_readable_field_t<f_vector_t>("adcs_monitor.ssa_vec");
            
            char buffer[3];

            for(int i = 0; i<20; i++){
                //ssa_voltages_fp.emplace_back(registry.find_readable_field_t<float>("adcs_monitor.ssa_voltage"+std::to_string(i)));
                ssa_voltages_fp.emplace_back(registry.find_readable_field_t<float>("adcs_monitor.ssa_voltage"+sprintf(buffer, "%u", i)));

            }

            mag_vec_fp = registry.find_readable_field_t<f_vector_t>("adcs_monitor.mag_vec");
            gyr_vec_fp = registry.find_readable_field_t<f_vector_t>("adcs_monitor.gyr_vec");
            gyr_temp_fp = registry.find_readable_field_t<float>("adcs_monitor.gyr_temp");

            assert(rwa_speed_rd_fp);
            assert(rwa_torque_rd_fp);
            assert(ssa_mode_fp);
            assert(ssa_vec_fp);

            for(int i = 0; i<20;i++)
                assert(ssa_voltages_fp[i]);
                
            assert(mag_vec_fp);
            assert(gyr_vec_fp);
            assert(gyr_temp_fp);
        
        }
};

float mag_2(const std::array<float, 3> input){
        return (float)(input[0]*input[0] + input[1]*input[1] + input[2]*input[2]);
}

void elements_same(const std::array<float, 3> ref, const std::array<float, 3> actual){
    TEST_ASSERT_FLOAT_WITHIN(0.001, ref[0], actual[0]);
    TEST_ASSERT_FLOAT_WITHIN(0.001, ref[1], actual[1]);
    TEST_ASSERT_FLOAT_WITHIN(0.001, ref[2], actual[2]);

}

void test_task_initialization()
{
    TestFixture tf;
}

void test_execute(){
    TestFixture tf;

    //mocking sets to max output
    std::array<float, 3> ref_rwa_max_speed = {rwa::max_speed_read, rwa::max_speed_read, rwa::max_speed_read};
    std::array<float, 3> ref_rwa_max_torque = {rwa::max_torque, rwa::max_torque, rwa::max_torque};
    std::array<float, 3> ref_three_unit = {1,1,1};
    std::array<float, 3> ref_mag_vec = {imu::max_rd_mag, imu::max_rd_mag, imu::max_rd_mag};
    std::array<float, 3> ref_gyr_vec = {imu::max_rd_omega, imu::max_rd_omega, imu::max_rd_omega};

    //call box monitor control task, to pull values using driver
    tf.adcs_box->execute();

    elements_same(ref_rwa_max_speed, tf.rwa_speed_rd_fp->get());
    elements_same(ref_rwa_max_torque, tf.rwa_torque_rd_fp->get());
    TEST_ASSERT_EQUAL(2, tf.ssa_mode_fp->get());
    elements_same(ref_three_unit, tf.ssa_vec_fp->get());

    for(int i = 0; i<20; i++){
        TEST_ASSERT_EQUAL(ssa::max_voltage_rd,tf.ssa_voltages_fp[i]->get());
    }

    elements_same(ref_mag_vec,tf.mag_vec_fp->get());
    elements_same(ref_gyr_vec, tf.gyr_vec_fp->get());
    TEST_ASSERT_EQUAL(imu::max_rd_temp, tf.gyr_temp_fp->get());

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