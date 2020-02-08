#include "../StateFieldRegistryMock.hpp"

#include <adcs/constants.hpp>
#include <adcs/havt_devices.hpp>
#include <fsw/FCCode/ADCSCommander.hpp>
#include <fsw/FCCode/Drivers/ADCS.hpp>

#include <unity.h>
#include "../custom_assertions.hpp"
class TestFixture {
    public:
        StateFieldRegistryMock registry;

        // Pointers to input statefields
        std::shared_ptr<WritableStateField<unsigned char>> adcs_state_fp;
        // std::vector<std::shared_ptr<WritableStateField<bool>>> havt_read_table_vector_fp;
        std::shared_ptr<WritableStateField<f_vector_t>> adcs_vec1_current_fp;
        std::shared_ptr<WritableStateField<f_vector_t>> adcs_vec1_desired_fp;
        std::shared_ptr<WritableStateField<f_vector_t>> adcs_vec2_current_fp;
        std::shared_ptr<WritableStateField<f_vector_t>> adcs_vec2_desired_fp;

        // Pointers to output statefields
        WritableStateField<unsigned char>* rwa_mode_fp;
        WritableStateField<f_vector_t>* rwa_speed_cmd_fp;
        WritableStateField<f_vector_t>* rwa_torque_cmd_fp;
        WritableStateField<float>* rwa_speed_filter_fp;
        WritableStateField<float>* rwa_ramp_filter_fp;
        WritableStateField<unsigned char>* mtr_mode_f;
        WritableStateField<f_vector_t>* mtr_cmd_f;
        WritableStateField<float>* mtr_limit_f;
        WritableStateField<float>* ssa_voltage_filter_f;
        WritableStateField<unsigned char>* imu_mode_f;
        WritableStateField<float>* imu_mag_filter_f;
        WritableStateField<float>* imu_gyr_filter_f;
        WritableStateField<float>* imu_gyr_temp_filter_f;
        WritableStateField<float>* imu_gyr_temp_kp_f;
        WritableStateField<float>* imu_gyr_temp_ki_f;
        WritableStateField<float>* imu_gyr_temp_kd_f;
        WritableStateField<float>* imu_gyr_temp_desired_f;

        std::vector<WritableStateField<bool>*> havt_cmd_table_vector_fp;

        /**
         * @brief If this statefield is true, then the cmd_table is uploaded to ADCS
         * 
         * Statefield is set to true for autonomous HAVT response, or by ground command
         */
        // TODO PROBAABLY DELETE
        WritableStateField<bool>* havt_cmd_apply_f;

        std::unique_ptr<ADCSCommander> adcs_cmder;
        
        // Create a TestFixture instance of ADCSCommander with pointers to statefields
        // Compile conditionally for either hootl or hitl
        TestFixture() : registry(){
            adcs_state_fp = registry.create_writable_field<unsigned char>("adcs.state", 8);

            adcs_vec1_current_fp = registry.create_writable_vector_field<float>("adcs.compute.vec1.current", 0, 1, 100);
            adcs_vec1_desired_fp = registry.create_writable_vector_field<float>("adcs.compute.vec1.desired", 0, 1, 100);
            adcs_vec2_current_fp = registry.create_writable_vector_field<float>("adcs.compute.vec2.current", 0, 1, 100);
            adcs_vec2_desired_fp = registry.create_writable_vector_field<float>("adcs.compute.vec2.desired", 0, 1, 100);

            adcs_cmder = std::make_unique<ADCSCommander>(registry, 0);  

            // initialize pointers to output statefields
            rwa_mode_fp = registry.find_writable_field_t<unsigned char>("adcs_cmd.rwa_mode");
            rwa_speed_cmd_fp = registry.find_writable_field_t<f_vector_t>("adcs_cmd.rwa_speed_cmd");
            rwa_torque_cmd_fp = registry.find_writable_field_t<f_vector_t>("adcs_cmd.rwa_torque_cmd");
            rwa_speed_filter_fp = registry.find_writable_field_t<float>("adcs_cmd.rwa_speed_filter");
            rwa_ramp_filter_fp = registry.find_writable_field_t<float>("adcs_cmd.rwa_ramp_filter");
            mtr_mode_f = registry.find_writable_field_t<unsigned char>("adcs_cmd.mtr_mode");
            mtr_cmd_f = registry.find_writable_field_t<f_vector_t>("adcs_cmd.mtr_cmd");
            mtr_limit_f = registry.find_writable_field_t<float>("adcs_cmd.mtr_limit");
            ssa_voltage_filter_f = registry.find_writable_field_t<float>("adcs_cmd.ssa_voltage_filter");
            imu_mode_f = registry.find_writable_field_t<unsigned char>("adcs_cmd.imu_mode");
            imu_mag_filter_f = registry.find_writable_field_t<float>("adcs_cmd.imu_mag_filter");
            imu_gyr_filter_f = registry.find_writable_field_t<float>("adcs_cmd.imu_gyr_filter");
            imu_gyr_temp_filter_f = registry.find_writable_field_t<float>("adcs_cmd.imu_gyr_temp_filter");
            imu_gyr_temp_kp_f = registry.find_writable_field_t<float>("adcs_cmd.imu_gyr_temp_kp");
            imu_gyr_temp_ki_f = registry.find_writable_field_t<float>("adcs_cmd.imu_gyr_temp_ki");
            imu_gyr_temp_kd_f = registry.find_writable_field_t<float>("adcs_cmd.imu_gyr_temp_kd");
            imu_gyr_temp_desired_f = registry.find_writable_field_t<float>("adcs_cmd.imu_gyr_temp_desired");

            //fill vector of pointers to output statefields for havt
            char buffer[50];
            for (unsigned int idx = adcs::havt::Index::IMU_GYR; idx < adcs::havt::Index::_LENGTH; idx++ )
            {
                std::memset(buffer, 0, sizeof(buffer));
                sprintf(buffer,"adcs_cmd.havt_device");
                sprintf(buffer + strlen(buffer), "%u", idx);
                havt_cmd_table_vector_fp.push_back(registry.find_writable_field_t<bool>(buffer));
            }
        }
};

void test_task_initialization()
{
    TestFixture tf;
    f_vector_t zeros{0,0,0};

    TEST_ASSERT_EQUAL(adcs::RWAMode::RWA_DISABLED, tf.rwa_mode_fp->get());
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(zeros, tf.rwa_speed_cmd_fp->get(), 0);
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(zeros, tf.rwa_torque_cmd_fp->get(), 0);
    TEST_ASSERT_FLOAT_WITHIN(1, tf.rwa_speed_filter_fp->get(), 0);
    TEST_ASSERT_FLOAT_WITHIN(1, tf.rwa_ramp_filter_fp->get(), 0);
    TEST_ASSERT_EQUAL(adcs::MTRMode::MTR_DISABLED, tf.mtr_mode_f->get());
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(zeros, tf.mtr_cmd_f->get(), 0);
    TEST_ASSERT_FLOAT_WITHIN(1, tf.mtr_limit_f->get(), 0);
    TEST_ASSERT_FLOAT_WITHIN(1, tf.ssa_voltage_filter_f->get(), 0);
    TEST_ASSERT_EQUAL(adcs::IMUMode::MAG1, tf.imu_mode_f->get());
    TEST_ASSERT_FLOAT_WITHIN(1, tf.imu_mag_filter_f->get(), 0);
    TEST_ASSERT_FLOAT_WITHIN(1, tf.imu_gyr_filter_f->get(), 0);
    TEST_ASSERT_FLOAT_WITHIN(1, tf.imu_gyr_temp_filter_f->get(), 0);
    TEST_ASSERT_FLOAT_WITHIN(1, tf.imu_gyr_temp_kp_f->get(), 0);
    TEST_ASSERT_FLOAT_WITHIN(1, tf.imu_gyr_temp_ki_f->get(), 0);
    TEST_ASSERT_FLOAT_WITHIN(1, tf.imu_gyr_temp_kd_f->get(), 0);
    TEST_ASSERT_FLOAT_WITHIN(20, tf.imu_gyr_temp_desired_f->get(), 0);

    // verify all initialized to 0
    for(unsigned int idx = adcs::havt::Index::IMU_GYR; idx < adcs::havt::Index::_LENGTH; idx++ )
    {
        // 0 means device is disabled
        // TODO EDIT DEF: 0 means don't apply and changes to HAVT table
        TEST_ASSERT_EQUAL(0, tf.havt_cmd_table_vector_fp[idx]->get());
    }
}

void test_execute(){
    TestFixture tf;

    //set all the states and watch em go
    //tf.adcs_state_fp->
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