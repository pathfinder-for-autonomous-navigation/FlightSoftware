#include "../StateFieldRegistryMock.hpp"

#include <ADCS.hpp>
#include "../../src/FCCode/ADCSBoxMonitor.hpp"
#include "adcs_constants.hpp"

#include <unity.h>

class TestFixture {
    public:
        StateFieldRegistryMock registry;

        // pointers to output statefields for easy access
        ReadableStateField<f_vector_t>* rwa_speed_rd_fp;
        ReadableStateField<f_vector_t>* rwa_torque_rd_fp;
        ReadableStateField<int>* ssa_mode_fp;
        ReadableStateField<f_vector_t>* ssa_vec_fp;
        std::vector<ReadableStateField<float>*> ssa_voltages_fp;
        ReadableStateField<f_vector_t>* mag_vec_fp;
        ReadableStateField<f_vector_t>* gyr_vec_fp;
        ReadableStateField<float>* gyr_temp_fp;

        // pointers to error flags
        ReadableStateField<bool>* rwa_speed_rd_flag_p;
        ReadableStateField<bool>* rwa_torque_rd_flag_p;
        ReadableStateField<bool>* ssa_vec_flag_p;
        ReadableStateField<bool>* mag_vec_flag_p;
        ReadableStateField<bool>* gyr_vec_flag_p;
        ReadableStateField<bool>* gyr_temp_flag_p;

        std::unique_ptr<ADCSBoxMonitor> adcs_box;

        Devices::ADCS adcs;
        
        // Create a TestFixture instance of AttitudeEstimator with pointers to statefields
        TestFixture() : registry(), adcs(){

            adcs_box = std::make_unique<ADCSBoxMonitor>(registry, 0, adcs);  

            // initialize pointers to statefields
            rwa_speed_rd_fp = registry.find_readable_field_t<f_vector_t>("adcs_monitor.rwa_speed_rd");
            rwa_torque_rd_fp = registry.find_readable_field_t<f_vector_t>("adcs_monitor.rwa_torque_rd");
            ssa_mode_fp = registry.find_readable_field_t<int>("adcs_monitor.ssa_mode");
            ssa_vec_fp = registry.find_readable_field_t<f_vector_t>("adcs_monitor.ssa_vec");
            
            // fill vector of pointers to statefields
            char buffer[50];
            for(unsigned int i = 0; i<ADCSBoxMonitor::num_sun_sensors; i++){
                std::memset(buffer, 0, sizeof(buffer));
                sprintf(buffer,"adcs_monitor.ssa_voltage");
                sprintf(buffer + strlen(buffer), "%u", i);
                ssa_voltages_fp.push_back(registry.find_readable_field_t<float>(buffer));
            }

            mag_vec_fp = registry.find_readable_field_t<f_vector_t>("adcs_monitor.mag_vec");
            gyr_vec_fp = registry.find_readable_field_t<f_vector_t>("adcs_monitor.gyr_vec");
            gyr_temp_fp = registry.find_readable_field_t<float>("adcs_monitor.gyr_temp");

            //find flag state fields
            rwa_speed_rd_flag_p = registry.find_readable_field_t<bool>("adcs_monitor.speed_rd_flag");
            rwa_torque_rd_flag_p = registry.find_readable_field_t<bool>("adcs_monitor.torque_rd_flag");
            ssa_vec_flag_p = registry.find_readable_field_t<bool>("adcs_monitor.ssa_vec_flag");
            mag_vec_flag_p = registry.find_readable_field_t<bool>("adcs_monitor.mag_vec_flag");
            gyr_vec_flag_p = registry.find_readable_field_t<bool>("adcs_monitor.gyr_vec_flag");
            gyr_temp_flag_p = registry.find_readable_field_t<bool>("adcs_monitor.gyr_temp_flag");

        }

        //set of mocking methods
        void set_mock_ssa_mode(const unsigned int mode){
                adcs_box->adcs_system.set_mock_ssa_mode(mode);
        }
};

//checks that all ref vector and actual vector are pretty much the same
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

    //define nan
    const float nan = std::numeric_limits<float>::quiet_NaN();

    //mocking sets to max output
    //see ADCS.cpp for mocking details
    std::array<float, 3> ref_rwa_max_speed = {rwa::max_speed_read, rwa::max_speed_read, rwa::max_speed_read};
    std::array<float, 3> ref_rwa_max_torque = {rwa::max_torque, rwa::max_torque, rwa::max_torque};
    std::array<float, 3> ref_three_unit = {1,1,1};
    std::array<float, 3> ref_mag_vec = {imu::max_rd_mag, imu::max_rd_mag, imu::max_rd_mag};
    std::array<float, 3> ref_gyr_vec = {imu::max_rd_omega, imu::max_rd_omega, imu::max_rd_omega};

    //set mock return to COMPLETE
    tf.set_mock_ssa_mode(SSAMode::SSA_COMPLETE);

    //call box monitor control task, to pull values using driver
    tf.adcs_box->execute();

    //verify that the values are read into statefields correctly
    elements_same(ref_rwa_max_speed, tf.rwa_speed_rd_fp->get());
    elements_same(ref_rwa_max_torque, tf.rwa_torque_rd_fp->get());
    TEST_ASSERT_EQUAL(SSAMode::SSA_COMPLETE, tf.ssa_mode_fp->get());
    elements_same(ref_three_unit, tf.ssa_vec_fp->get());

    for(unsigned int i = 0; i<ADCSBoxMonitor::num_sun_sensors; i++){
        TEST_ASSERT_EQUAL(ssa::max_voltage_rd,tf.ssa_voltages_fp[i]->get());
    }

    elements_same(ref_mag_vec,tf.mag_vec_fp->get());
    elements_same(ref_gyr_vec, tf.gyr_vec_fp->get());
    TEST_ASSERT_EQUAL(imu::max_rd_temp, tf.gyr_temp_fp->get());

    //verify that all flags are set to true
    //since temp bounds are all max - 1
    //mocking using max output sets all flags to true
    TEST_ASSERT_TRUE(tf.rwa_speed_rd_flag_p->get());
    TEST_ASSERT_TRUE(tf.rwa_torque_rd_flag_p->get());
    TEST_ASSERT_TRUE(tf.ssa_vec_flag_p->get());
    TEST_ASSERT_TRUE(tf.mag_vec_flag_p->get());
    TEST_ASSERT_TRUE(tf.gyr_vec_flag_p->get());
    TEST_ASSERT_TRUE(tf.gyr_temp_flag_p->get());

    //TEST IN_PROGRESS
    //set mock return to IN_PROGRESS
    tf.set_mock_ssa_mode(SSAMode::SSA_IN_PROGRESS);

    //call box monitor control task, to pull values using driver
    tf.adcs_box->execute();

    //verify that the values are read into statefields correctly
    elements_same(ref_rwa_max_speed, tf.rwa_speed_rd_fp->get());
    elements_same(ref_rwa_max_torque, tf.rwa_torque_rd_fp->get());
    TEST_ASSERT_EQUAL(SSAMode::SSA_IN_PROGRESS, tf.ssa_mode_fp->get());

    //test ssa_vec is nan
    TEST_ASSERT_EQUAL(nan, tf.ssa_vec_fp->get()[0]);
    TEST_ASSERT_EQUAL(nan, tf.ssa_vec_fp->get()[1]);
    TEST_ASSERT_EQUAL(nan, tf.ssa_vec_fp->get()[2]);

    for(unsigned int i = 0; i<ADCSBoxMonitor::num_sun_sensors; i++){
        TEST_ASSERT_EQUAL(ssa::max_voltage_rd,tf.ssa_voltages_fp[i]->get());
    }

    elements_same(ref_mag_vec,tf.mag_vec_fp->get());
    elements_same(ref_gyr_vec, tf.gyr_vec_fp->get());
    TEST_ASSERT_EQUAL(imu::max_rd_temp, tf.gyr_temp_fp->get());

    //verify that all flags are set to true
    //since temp bounds are all max - 1
    //mocking using max output sets all flags to true
    TEST_ASSERT_TRUE(tf.rwa_speed_rd_flag_p->get());
    TEST_ASSERT_TRUE(tf.rwa_torque_rd_flag_p->get());
    TEST_ASSERT_TRUE(tf.ssa_vec_flag_p->get());
    TEST_ASSERT_TRUE(tf.mag_vec_flag_p->get());
    TEST_ASSERT_TRUE(tf.gyr_vec_flag_p->get());
    TEST_ASSERT_TRUE(tf.gyr_temp_flag_p->get());

    //TEST FAILURE
    //set mock return to FAILURE
    tf.set_mock_ssa_mode(SSAMode::SSA_FAILURE);

    //call box monitor control task, to pull values using driver
    tf.adcs_box->execute();

    //verify that the values are read into statefields correctly
    elements_same(ref_rwa_max_speed, tf.rwa_speed_rd_fp->get());
    elements_same(ref_rwa_max_torque, tf.rwa_torque_rd_fp->get());
    TEST_ASSERT_EQUAL(SSAMode::SSA_FAILURE, tf.ssa_mode_fp->get());
    
    //test ssa_vec is nan
    TEST_ASSERT_EQUAL(nan, tf.ssa_vec_fp->get()[0]);
    TEST_ASSERT_EQUAL(nan, tf.ssa_vec_fp->get()[1]);
    TEST_ASSERT_EQUAL(nan, tf.ssa_vec_fp->get()[2]);


    for(unsigned int i = 0; i<ADCSBoxMonitor::num_sun_sensors; i++){
        TEST_ASSERT_EQUAL(ssa::max_voltage_rd,tf.ssa_voltages_fp[i]->get());
    }

    elements_same(ref_mag_vec,tf.mag_vec_fp->get());
    elements_same(ref_gyr_vec, tf.gyr_vec_fp->get());
    TEST_ASSERT_EQUAL(imu::max_rd_temp, tf.gyr_temp_fp->get());

    //verify that all flags are set to true
    //since temp bounds are all max - 1
    //mocking using max output sets all flags to true
    TEST_ASSERT_TRUE(tf.rwa_speed_rd_flag_p->get());
    TEST_ASSERT_TRUE(tf.rwa_torque_rd_flag_p->get());
    TEST_ASSERT_TRUE(tf.ssa_vec_flag_p->get());
    TEST_ASSERT_TRUE(tf.mag_vec_flag_p->get());
    TEST_ASSERT_TRUE(tf.gyr_vec_flag_p->get());
    TEST_ASSERT_TRUE(tf.gyr_temp_flag_p->get());
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