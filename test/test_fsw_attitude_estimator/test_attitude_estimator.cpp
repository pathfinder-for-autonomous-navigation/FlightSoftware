#include "../StateFieldRegistryMock.hpp"

#include <fsw/FCCode/AttitudeEstimator.hpp>

#include <unity.h>
#include <fsw/FCCode/radio_state_t.enum>
class TestFixture {
    public:
        StateFieldRegistryMock registry;
        std::shared_ptr<InternalStateField<unsigned char>> radio_state_fp;

        // pointers to input statefields
        std::shared_ptr<ReadableStateField<gps_time_t>> piksi_time_fp;
        std::shared_ptr<ReadableStateField<d_vector_t>> pos_vec_ecef_fp;
        std::shared_ptr<ReadableStateField<f_vector_t>> ssa_vec_rd_fp;
        std::shared_ptr<ReadableStateField<f_vector_t>> mag_vec_fp;

        // pointers to output statefields for easy access
        ReadableStateField<f_quat_t>* q_body_eci_fp;
        ReadableStateField<f_vector_t>* w_body_fp;
        InternalStateField<lin::Vector3f>* h_body_fp;
        WritableStateField<bool>* adcs_paired_fp;
        WritableStateField<bool>* data_deaf_fp;

        std::unique_ptr<AttitudeEstimator> attitude_estimator;

        // Create a TestFixture instance of AttitudeEstimator with pointers to statefields
        TestFixture() : registry(){
            radio_state_fp = registry.create_internal_field<unsigned char>("radio.state");
            // by default, set the state to something that does not interfere with piksi
            radio_state_fp->set(static_cast<unsigned char>(radio_state_t::config));
            
            //create input statefields
            piksi_time_fp = registry.create_readable_field<gps_time_t>("piksi.time");
            pos_vec_ecef_fp = registry.create_readable_vector_field<double>("piksi.pos",0.0L,1000000.0L,64*3);
            ssa_vec_rd_fp = registry.create_readable_vector_field<float>("adcs_monitor.ssa_vec",-1.0,1.0,32*3),
            mag_vec_fp = registry.create_readable_vector_field<float>("adcs_monitor.mag_vec",-16e-4,16e4,32*3),

            attitude_estimator = std::make_unique<AttitudeEstimator>(registry, 0);  

            // initialize pointers to statefields
            q_body_eci_fp = registry.find_readable_field_t<f_quat_t>("attitude_estimator.q_body_eci");
            w_body_fp = registry.find_readable_field_t<f_vector_t>("attitude_estimator.w_body");
            h_body_fp = registry.find_internal_field_t<lin::Vector3f>("attitude_estimator.h_body");
            adcs_paired_fp = registry.find_writable_field_t<bool>("adcs.paired");
            data_deaf_fp = registry.find_writable_field_t<bool>("attitude_estimator.deaf");
        }
};

void test_task_initialization()
{
    TestFixture tf;
}

void test_execute(){
    TestFixture tf;

    tf.piksi_time_fp->set(gps_time_t(0,420,420));
    // assuming 500km orbit
    tf.pos_vec_ecef_fp->set({500000.0L,0.0L,0.0L});
    tf.ssa_vec_rd_fp->set({0.6,0.8,0.0});
    tf.mag_vec_fp->set({0.0001,0.0001,0.0001});

    for(int i = 0; i < 100; i++){
        tf.piksi_time_fp->set(gps_time_t(0,420+i*120,420));
        tf.attitude_estimator->execute();
    }

    TEST_ASSERT_FLOAT_IS_NOT_NAN(tf.q_body_eci_fp->get()[0]);
    TEST_ASSERT_FLOAT_IS_NOT_NAN(tf.q_body_eci_fp->get()[1]);
    TEST_ASSERT_FLOAT_IS_NOT_NAN(tf.q_body_eci_fp->get()[2]);
    TEST_ASSERT_FLOAT_IS_NOT_NAN(tf.q_body_eci_fp->get()[3]);
    TEST_ASSERT_FLOAT_IS_NOT_NAN(tf.w_body_fp->get()[0]);
    TEST_ASSERT_FLOAT_IS_NOT_NAN(tf.w_body_fp->get()[1]);
    TEST_ASSERT_FLOAT_IS_NOT_NAN(tf.w_body_fp->get()[2]);
    TEST_ASSERT_FLOAT_IS_NOT_NAN(tf.h_body_fp->get()(0));
    TEST_ASSERT_FLOAT_IS_NOT_NAN(tf.h_body_fp->get()(1));
    TEST_ASSERT_FLOAT_IS_NOT_NAN(tf.h_body_fp->get()(2));
}
void test_data_deaf(){
    TestFixture tf;

    tf.piksi_time_fp->set(gps_time_t(0,0,0));
    // assuming 500km orbit
    tf.pos_vec_ecef_fp->set({500000.0L,0.0L,0.0L});
    tf.ssa_vec_rd_fp->set({0.6,0.8,0.0});
    tf.mag_vec_fp->set({0.0001,0.0001,0.0001});

    // set radio to transceive
    tf.radio_state_fp->set(static_cast<unsigned char>(radio_state_t::transceive));  

    for(int i = 0; i < 100; i++){
        tf.piksi_time_fp->set(gps_time_t(0,i*120,0));
        tf.attitude_estimator->execute();
    }
    
    TEST_ASSERT_FLOAT_IS_NAN(tf.q_body_eci_fp->get()[0]);
    TEST_ASSERT_FLOAT_IS_NAN(tf.q_body_eci_fp->get()[1]);
    TEST_ASSERT_FLOAT_IS_NAN(tf.q_body_eci_fp->get()[2]);
    TEST_ASSERT_FLOAT_IS_NAN(tf.q_body_eci_fp->get()[3]);
    TEST_ASSERT_FLOAT_IS_NAN(tf.w_body_fp->get()[0]);
    TEST_ASSERT_FLOAT_IS_NAN(tf.w_body_fp->get()[1]);
    TEST_ASSERT_FLOAT_IS_NAN(tf.w_body_fp->get()[2]);
    TEST_ASSERT_FLOAT_IS_NAN(tf.h_body_fp->get()(0));
    TEST_ASSERT_FLOAT_IS_NAN(tf.h_body_fp->get()(1));
    TEST_ASSERT_FLOAT_IS_NAN(tf.h_body_fp->get()(2));

    // suppress deaf condition
    tf.data_deaf_fp->set(false);
    for(int i = 0; i < 100; i++){
        tf.piksi_time_fp->set(gps_time_t(0,120*100+i*120,0));
        tf.attitude_estimator->execute();
    }
    
    TEST_ASSERT_FLOAT_IS_NOT_NAN(tf.q_body_eci_fp->get()[0]);
    TEST_ASSERT_FLOAT_IS_NOT_NAN(tf.q_body_eci_fp->get()[1]);
    TEST_ASSERT_FLOAT_IS_NOT_NAN(tf.q_body_eci_fp->get()[2]);
    TEST_ASSERT_FLOAT_IS_NOT_NAN(tf.q_body_eci_fp->get()[3]);
    TEST_ASSERT_FLOAT_IS_NOT_NAN(tf.w_body_fp->get()[0]);
    TEST_ASSERT_FLOAT_IS_NOT_NAN(tf.w_body_fp->get()[1]);
    TEST_ASSERT_FLOAT_IS_NOT_NAN(tf.w_body_fp->get()[2]);
    TEST_ASSERT_FLOAT_IS_NOT_NAN(tf.h_body_fp->get()(0));
    TEST_ASSERT_FLOAT_IS_NOT_NAN(tf.h_body_fp->get()(1));
    TEST_ASSERT_FLOAT_IS_NOT_NAN(tf.h_body_fp->get()(2));
}

int test_control_task()
{
    UNITY_BEGIN();
    RUN_TEST(test_task_initialization);
    RUN_TEST(test_execute);
    RUN_TEST(test_data_deaf);
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