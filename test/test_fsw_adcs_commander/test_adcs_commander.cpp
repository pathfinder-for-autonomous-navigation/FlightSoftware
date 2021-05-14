#include "../StateFieldRegistryMock.hpp"

#include <adcs/constants.hpp>
#include <adcs/havt_devices.hpp>
#include <fsw/FCCode/ADCSCommander.hpp>
#include <fsw/FCCode/Drivers/ADCS.hpp>

#include "../custom_assertions.hpp"
#include "../custom_assertions.hpp"

#include <fsw/FCCode/adcs_state_t.enum>
class TestFixture {
    public:
        StateFieldRegistryMock registry;

        // Pointers to input statefields
        std::shared_ptr<WritableStateField<unsigned char>> adcs_state_fp;
        std::shared_ptr<WritableStateField<lin::Vector3f>> pointer_rwa_torque_cmd_fp;
        std::shared_ptr<WritableStateField<lin::Vector3f>> pointer_mtr_cmd_fp;

        // Pointers to output statefields
        WritableStateField<unsigned char>* rwa_mode_fp;
        WritableStateField<f_vector_t>* rwa_speed_cmd_fp;
        WritableStateField<f_vector_t>* rwa_torque_cmd_fp;
        WritableStateField<float>* rwa_speed_filter_fp;
        WritableStateField<float>* rwa_ramp_filter_fp;
        WritableStateField<unsigned char>* mtr_mode_fp;
        WritableStateField<f_vector_t>* mtr_cmd_fp;
        WritableStateField<float>* mtr_limit_fp;
        WritableStateField<float>* ssa_voltage_filter_fp;
        WritableStateField<unsigned char>* mag1_mode_fp;
        WritableStateField<unsigned char>* mag2_mode_fp;
        WritableStateField<float>* imu_mag_filter_fp;
        WritableStateField<float>* imu_gyr_filter_fp;
        WritableStateField<float>* imu_gyr_temp_filter_fp;
        WritableStateField<unsigned char>* imu_gyr_temp_pwm_fp;
        WritableStateField<float>* imu_gyr_temp_desired_fp;

        std::vector<WritableStateField<bool>*> havt_cmd_reset_vector_fp;
        std::vector<WritableStateField<bool>*> havt_cmd_disable_vector_fp;

        std::unique_ptr<ADCSCommander> adcs_cmder;
        
        // Create a TestFixture instance of ADCSCommander with pointers to statefields
        // Compile conditionally for either hootl or hitl
        TestFixture() : registry(){
            adcs_state_fp = registry.create_writable_field<unsigned char>("adcs.state", 8);

            pointer_rwa_torque_cmd_fp = registry.create_writable_lin_vector_field<float>(
                "pointer.rwa_torque_cmd", adcs::rwa::min_torque, adcs::rwa::max_torque, 16*3);
            pointer_mtr_cmd_fp        = registry.create_writable_lin_vector_field<float>(
                "pointer.mtr_cmd", adcs::mtr::min_moment, adcs::mtr::max_moment, 16*3);

            adcs_cmder = std::make_unique<ADCSCommander>(registry);  

            // initialize pointers to output statefields
            rwa_mode_fp = registry.find_writable_field_t<unsigned char>("adcs_cmd.rwa_mode");
            rwa_speed_cmd_fp = registry.find_writable_field_t<f_vector_t>("adcs_cmd.rwa_speed_cmd");
            rwa_torque_cmd_fp = registry.find_writable_field_t<f_vector_t>("adcs_cmd.rwa_torque_cmd");
            rwa_speed_filter_fp = registry.find_writable_field_t<float>("adcs_cmd.rwa_speed_filter");
            rwa_ramp_filter_fp = registry.find_writable_field_t<float>("adcs_cmd.rwa_ramp_filter");
            mtr_mode_fp = registry.find_writable_field_t<unsigned char>("adcs_cmd.mtr_mode");
            mtr_cmd_fp = registry.find_writable_field_t<f_vector_t>("adcs_cmd.mtr_cmd");
            mtr_limit_fp = registry.find_writable_field_t<float>("adcs_cmd.mtr_limit");
            ssa_voltage_filter_fp = registry.find_writable_field_t<float>("adcs_cmd.ssa_voltage_filter");
            mag1_mode_fp = registry.find_writable_field_t<unsigned char>("adcs_cmd.mag1_mode");
            mag2_mode_fp = registry.find_writable_field_t<unsigned char>("adcs_cmd.mag2_mode");
            imu_mag_filter_fp = registry.find_writable_field_t<float>("adcs_cmd.imu_mag_filter");
            imu_gyr_filter_fp = registry.find_writable_field_t<float>("adcs_cmd.imu_gyr_filter");
            imu_gyr_temp_filter_fp = registry.find_writable_field_t<float>("adcs_cmd.imu_gyr_temp_filter");
            imu_gyr_temp_pwm_fp = registry.find_writable_field_t<unsigned char>("adcs_cmd.imu_gyr_temp_pwm");
            imu_gyr_temp_desired_fp = registry.find_writable_field_t<float>("adcs_cmd.imu_gyr_temp_desired");

            havt_cmd_reset_vector_fp.reserve(adcs::havt::Index::_LENGTH);
            havt_cmd_disable_vector_fp.reserve(adcs::havt::Index::_LENGTH);
            //fill vector of pointers to output statefields for havt
            char buffer[50];
            for (unsigned int idx = adcs::havt::Index::IMU_GYR; idx < adcs::havt::Index::_LENGTH; idx++ )
            {
                std::memset(buffer, 0, sizeof(buffer));
                sprintf(buffer,"adcs_cmd.havt_reset");
                sprintf(buffer + strlen(buffer), "%u", idx);
                havt_cmd_reset_vector_fp.emplace_back(registry.find_writable_field_t<bool>(buffer));
            }
            for (unsigned int idx = adcs::havt::Index::IMU_GYR; idx < adcs::havt::Index::_LENGTH; idx++ )
            {
                std::memset(buffer, 0, sizeof(buffer));
                sprintf(buffer,"adcs_cmd.havt_disable");
                sprintf(buffer + strlen(buffer), "%u", idx);
                havt_cmd_disable_vector_fp.emplace_back(registry.find_writable_field_t<bool>(buffer));
            }
        }
        void set_adcs_state(adcs_state_t state){
            adcs_state_fp->set(static_cast<unsigned char>(state));
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
    TEST_ASSERT_EQUAL(adcs::MTRMode::MTR_DISABLED, tf.mtr_mode_fp->get());
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(zeros, tf.mtr_cmd_fp->get(), 0);
    TEST_ASSERT_FLOAT_WITHIN(1, tf.mtr_limit_fp->get(), 0);
    TEST_ASSERT_FLOAT_WITHIN(1, tf.ssa_voltage_filter_fp->get(), 0);
    TEST_ASSERT_EQUAL(adcs::IMU_MAG_NORMAL, tf.mag1_mode_fp->get());
    TEST_ASSERT_EQUAL(adcs::IMU_MAG_NORMAL, tf.mag2_mode_fp->get());
    TEST_ASSERT_FLOAT_WITHIN(1, tf.imu_mag_filter_fp->get(), 0);
    TEST_ASSERT_FLOAT_WITHIN(1, tf.imu_gyr_filter_fp->get(), 0);
    TEST_ASSERT_FLOAT_WITHIN(1, tf.imu_gyr_temp_filter_fp->get(), 0);
    TEST_ASSERT_EQUAL(tf.imu_gyr_temp_pwm_fp->get(), 255);
    TEST_ASSERT_FLOAT_WITHIN(20, tf.imu_gyr_temp_desired_fp->get(), 0);

    // verify all initialized to false
    for(unsigned int idx = adcs::havt::Index::IMU_GYR; idx < adcs::havt::Index::_LENGTH; idx++ )
    {
        // false means do not apply any commands
        TEST_ASSERT_EQUAL(false, tf.havt_cmd_reset_vector_fp[idx]->get());
        TEST_ASSERT_EQUAL(false, tf.havt_cmd_disable_vector_fp[idx]->get());
    }
}

void test_execute(){
    TestFixture tf;

    // Expand the sections once each dispatch block does specific things

    // in startup, execute() does nothing
    tf.set_adcs_state(adcs_state_t::startup);
    
    // set a hypothetical command that comes form the attitude controller
    tf.pointer_rwa_torque_cmd_fp->set({1,2,-3});
    tf.pointer_mtr_cmd_fp->set({4,5,-6});
    tf.adcs_cmder->execute();

    // verify that they are not translated to the output
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,0}), tf.rwa_torque_cmd_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,0}), tf.mtr_cmd_fp->get(), 1e-10);

    // manual should also do nothing
    // in startup, execute() does nothing
    tf.set_adcs_state(adcs_state_t::manual);
    tf.adcs_cmder->execute();

    // verify that they are not translated to the output
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,0}), tf.rwa_torque_cmd_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,0}), tf.mtr_cmd_fp->get(), 1e-10);


    // now directly set outputs as if it was a ptest case
    tf.set_adcs_state(adcs_state_t::manual);
    tf.rwa_mode_fp->set(adcs::RWAMode::RWA_ACCEL_CTRL);
    tf.mtr_mode_fp->set(adcs::MTRMode::MTR_ENABLED);
    tf.rwa_torque_cmd_fp->set({1,2,-3});
    tf.mtr_cmd_fp->set({4,5,-6});
    tf.adcs_cmder->execute();
    TEST_ASSERT_EQUAL(adcs::RWAMode::RWA_ACCEL_CTRL, tf.rwa_mode_fp->get());
    TEST_ASSERT_EQUAL(adcs::MTRMode::MTR_ENABLED, tf.mtr_mode_fp->get());
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({1,2,-3}), tf.rwa_torque_cmd_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({4,5,-6}), tf.mtr_cmd_fp->get(), 1e-10);

    tf.set_adcs_state(adcs_state_t::limited);
    tf.adcs_cmder->execute();
    TEST_ASSERT_EQUAL(adcs::RWAMode::RWA_DISABLED, tf.rwa_mode_fp->get());
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,0}), tf.rwa_torque_cmd_fp->get(), 1e-10);
    TEST_ASSERT_EQUAL(adcs::MTRMode::MTR_ENABLED, tf.mtr_mode_fp->get());
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({4,5,-6}), tf.mtr_cmd_fp->get(), 1e-10);

    tf.set_adcs_state(adcs_state_t::detumble);
    tf.adcs_cmder->execute();
    TEST_ASSERT_EQUAL(adcs::RWAMode::RWA_DISABLED, tf.rwa_mode_fp->get());
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,0}), tf.rwa_torque_cmd_fp->get(), 1e-10);
    TEST_ASSERT_EQUAL(adcs::MTRMode::MTR_ENABLED, tf.mtr_mode_fp->get());
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({4,5,-6}), tf.mtr_cmd_fp->get(), 1e-10);

    // point manual is a fully controlled state from the commander's perspective just like standby
    tf.set_adcs_state(adcs_state_t::point_manual);
    tf.adcs_cmder->execute();
    TEST_ASSERT_EQUAL(adcs::RWAMode::RWA_ACCEL_CTRL, tf.rwa_mode_fp->get());
    TEST_ASSERT_EQUAL(adcs::MTRMode::MTR_ENABLED, tf.mtr_mode_fp->get());
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({1,2,-3}), tf.rwa_torque_cmd_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({4,5,-6}), tf.mtr_cmd_fp->get(), 1e-10);

    tf.set_adcs_state(adcs_state_t::zero_torque);
    tf.adcs_cmder->execute();
    TEST_ASSERT_EQUAL(adcs::RWAMode::RWA_ACCEL_CTRL, tf.rwa_mode_fp->get());
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,0}), tf.rwa_torque_cmd_fp->get(), 1e-10);
    TEST_ASSERT_EQUAL(adcs::MTRMode::MTR_DISABLED, tf.mtr_mode_fp->get());
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,0}), tf.mtr_cmd_fp->get(), 1e-10);

    tf.set_adcs_state(adcs_state_t::zero_L);
    tf.adcs_cmder->execute();
    TEST_ASSERT_EQUAL(adcs::RWAMode::RWA_DISABLED, tf.rwa_mode_fp->get());
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,0}), tf.rwa_speed_cmd_fp->get(), 1e-10);    
    TEST_ASSERT_EQUAL(adcs::MTRMode::MTR_ENABLED, tf.mtr_mode_fp->get());    
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({4,5,-6}), tf.mtr_cmd_fp->get(), 1e-10);

    tf.set_adcs_state(adcs_state_t::point_standby);
    tf.adcs_cmder->execute();
    TEST_ASSERT_EQUAL(adcs::RWAMode::RWA_ACCEL_CTRL, tf.rwa_mode_fp->get());
    TEST_ASSERT_EQUAL(adcs::MTRMode::MTR_ENABLED, tf.mtr_mode_fp->get());
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({1,2,-3}), tf.rwa_torque_cmd_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({4,5,-6}), tf.mtr_cmd_fp->get(), 1e-10);

    tf.set_adcs_state(adcs_state_t::point_docking);
    tf.adcs_cmder->execute();
    TEST_ASSERT_EQUAL(adcs::RWAMode::RWA_ACCEL_CTRL, tf.rwa_mode_fp->get());
    TEST_ASSERT_EQUAL(adcs::MTRMode::MTR_ENABLED, tf.mtr_mode_fp->get());
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({1,2,-3}), tf.rwa_torque_cmd_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({4,5,-6}), tf.mtr_cmd_fp->get(), 1e-10);

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