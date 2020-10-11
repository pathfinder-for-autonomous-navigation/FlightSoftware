#define LIN_DESKTOP

#include "../StateFieldRegistryMock.hpp"
#include "../custom_assertions.hpp"

#include <fsw/FCCode/AttitudeController.hpp>
#include <fsw/FCCode/adcs_state_t.enum>

#include "../custom_assertions.hpp"
#include <gnc/constants.hpp>
#include <adcs/constants.hpp>

constexpr float nan_f = std::numeric_limits<float>::quiet_NaN();
const lin::Vector3f nan_vector = lin::Vector3f({nan_f, nan_f, nan_f});

class TestFixture {
  public:
    StateFieldRegistryMock registry;
    
    // Input state fields to attitude controller
    std::shared_ptr<ReadableStateField<lin::Vector3f>> b_body_rd_fp;
    std::shared_ptr<ReadableStateField<lin::Vector3f>> w_wheels_rd_fp;
    std::shared_ptr<ReadableStateField<lin::Vector3f>> b_body_est_fp;
    std::shared_ptr<ReadableStateField<lin::Vector3f>> s_body_est_fp;
    std::shared_ptr<ReadableStateField<lin::Vector4f>> q_body_eci_est_fp;
    std::shared_ptr<ReadableStateField<lin::Vector3f>> w_body_est_fp;
    std::shared_ptr<WritableStateField<unsigned char>> adcs_state_fp;
    std::shared_ptr<ReadableStateField<unsigned int>>  time_ns_fp;
    std::shared_ptr<ReadableStateField<lin::Vector3d>> pos_ecef_fp;
    std::shared_ptr<ReadableStateField<lin::Vector3d>> vel_ecef_fp;
    std::shared_ptr<ReadableStateField<lin::Vector3d>> pos_baseline_ecef_fp;

    // Attitude controller
    std::unique_ptr<AttitudeController> attitude_controller;
    
    // Output state fields
    ReadableStateField<lin::Vector3f>* pointer_vec1_current_fp;
    WritableStateField<lin::Vector3f>* pointer_vec1_desired_fp;
    ReadableStateField<lin::Vector3f>* pointer_vec2_current_fp;
    WritableStateField<lin::Vector3f>* pointer_vec2_desired_fp;
    WritableStateField<f_vector_t>*    t_body_cmd_fp;
    WritableStateField<f_vector_t>*    m_body_cmd_fp;

    TestFixture() : registry() {
        b_body_rd_fp = registry.create_readable_lin_vector_field<float>("adcs_monitor.mag_vec", -1, 1, 100);
        w_wheels_rd_fp = registry.create_readable_lin_vector_field<float>("adcs_monitor.rwa_speed_rd", 0, 1, 100);
        b_body_est_fp = registry.create_readable_lin_vector_field<float>("attitude_estimator.b_body", 0, 1, 100);
        s_body_est_fp = registry.create_readable_lin_vector_field<float>("attitude_estimator.s_body", 0, 1, 100);
        q_body_eci_est_fp = registry.create_readable_field<lin::Vector4f>("attitude_estimator.q_body_eci");
        w_body_est_fp = registry.create_readable_lin_vector_field<float>("attitude_estimator.w_body", 0, 1, 100);
        adcs_state_fp = registry.create_writable_field<unsigned char>("adcs.state");
        time_ns_fp = registry.create_readable_field<unsigned int>("orbit.time");
        pos_ecef_fp = registry.create_readable_lin_vector_field<double>("orbit.pos_ecef", 0, 1, 100);
        vel_ecef_fp = registry.create_readable_lin_vector_field<double>("orbit.vel_ecef", 0, 1, 100);
        pos_baseline_ecef_fp = registry.create_readable_lin_vector_field<double>("orbit.pos_baseline_ecef", 0, 1, 100);
        
        // adcs_state_fp = registry.create_writable_field<unsigned char>("adcs.state", 8);
        // q_body_eci_fp = registry.create_readable_field<lin::Vector4f>("attitude_estimator.q_body_eci");
        // s_body_est_fp = registry.create_readable_lin_vector_field<float>("adcs_monitor.ssa_vec", 0, 1, 100);
        // pos_fp = registry.create_readable_lin_vector_field<double>("orbit.pos", 0, 100000, 100);
        // pos_baseline_fp = registry.create_readable_lin_vector_field<double>("orbit.baseline_pos", 0, 100000, 100);

        attitude_controller = std::make_unique<AttitudeController>(registry, 0);

        // Check that attitude controller creates its expected fields
        pointer_vec1_current_fp = registry.find_readable_field_t<lin::Vector3f>("attitude.pointer_vec1_current");
        pointer_vec2_current_fp = registry.find_readable_field_t<lin::Vector3f>("attitude.pointer_vec2_current");
        pointer_vec1_desired_fp = registry.find_writable_field_t<lin::Vector3f>("attitude.pointer_vec1_desired");
        pointer_vec2_desired_fp = registry.find_writable_field_t<lin::Vector3f>("attitude.pointer_vec2_desired");
        t_body_cmd_fp = registry.find_writable_field_t<f_vector_t>("adcs_cmd.rwa_torque_cmd");
        m_body_cmd_fp = registry.find_writable_field_t<f_vector_t>("adcs_cmd.mtr_cmd");

        // Set quaternion to non-rotating value
        q_body_eci_est_fp->set(lin::Vector4f({0,0,0,1}));
    }

    void step(){
        attitude_controller->execute();
    }
};

void load_good_data(TestFixture& tf){
    tf.b_body_est_fp->set(lin::Vector3f({1,-1,0}));
    tf.w_body_est_fp->set(lin::Vector3f({10,-1,0}));
    tf.pos_ecef_fp->set(lin::Vector3f({(6371+400)*1000,0,0}));
    tf.vel_ecef_fp->set(lin::Vector3f({0,7650,0}));
    tf.q_body_eci_est_fp->set(lin::Vector4f({0,0,0,1}));
    tf.time_ns_fp->set(0);
    tf.pos_baseline_ecef_fp->set(lin::Vector3f({500,1,0}));
}

void nan_sensors(TestFixture& tf){
    tf.b_body_est_fp->set(lin::nans<lin::Vector3f>());
    tf.w_body_est_fp->set(lin::nans<lin::Vector3f>());
    tf.pos_ecef_fp->set(lin::nans<lin::Vector3f>());
    tf.vel_ecef_fp->set(lin::nans<lin::Vector3f>());
    tf.q_body_eci_est_fp->set(lin::nans<lin::Vector4f>());
    tf.time_ns_fp->set(0);
    tf.pos_baseline_ecef_fp->set(lin::nans<lin::Vector3f>());
}

void test_valid_initialization() {
    TestFixture tf;

    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(nan_vector, tf.pointer_vec1_current_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(nan_vector, tf.pointer_vec2_current_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(nan_vector, tf.pointer_vec1_desired_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(nan_vector, tf.pointer_vec2_desired_fp->get(), 1e-10);

    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,0}), tf.t_body_cmd_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,0}), tf.m_body_cmd_fp->get(), 1e-10);
}

void assert_pointing_vectors_nan(TestFixture& tf){
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(nan_vector, tf.pointer_vec1_current_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(nan_vector, tf.pointer_vec2_current_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(nan_vector, tf.pointer_vec1_desired_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(nan_vector, tf.pointer_vec2_desired_fp->get(), 1e-10);
}

void test_detumble(){
    TestFixture tf;
    
    tf.adcs_state_fp->set(static_cast<unsigned char>(adcs_state_t::detumble));
    tf.b_body_rd_fp->set(lin::Vector3f({1,-1,0}));
    tf.step();
    
    // all things related to the pointing objectives should be NaN
    assert_pointing_vectors_nan(tf);

    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,0}), tf.t_body_cmd_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,0}), tf.m_body_cmd_fp->get(), 1e-10);

    // dump in 8 data points to almost fill the buffer
    for(int i = 0; i<8; i++){
        tf.b_body_rd_fp->set(lin::Vector3f({1,-1,0}));
        tf.step();
    }

    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,0}), tf.m_body_cmd_fp->get(), 1e-10);

    // last data point, now size = 10 and we should have a non zero mtr actuation
    tf.b_body_rd_fp->set(lin::Vector3f({-1,1,0}));
    tf.step();

    // check that we are able to dump into the mtr output command
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t(
        {adcs::mtr::max_moment,-adcs::mtr::max_moment,0}),
        tf.m_body_cmd_fp->get(), 1e-10);

    // check that if we dont have mag readings actuators go to 0
    tf.b_body_rd_fp->set(lin::nans<lin::Vector3f>());
    tf.step();
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,0}), tf.t_body_cmd_fp->get(), 1e-10);    
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,0}), tf.m_body_cmd_fp->get(), 1e-10);

}

void test_standby(){
    TestFixture tf;
    tf.adcs_state_fp->set(static_cast<unsigned char>(adcs_state_t::point_standby));
    load_good_data(tf);

    tf.step();

    std::cout << tf.pointer_vec1_desired_fp->get();
    std::cout << tf.pointer_vec2_desired_fp->get();

    // all things related to the pointing objectives should be NaN
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(lin::Vector3f({1.0f, 0.0f, 0.0f}), tf.pointer_vec1_current_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(lin::Vector3f({0.0f, 0.0f, 1.0f}), tf.pointer_vec2_current_fp->get(), 1e-10);
    
    // this test is doomed to pass, but the important part is that it is not nan
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(lin::Vector3f({-0.105f, -0.994f, 0.00017f}), tf.pointer_vec1_desired_fp->get(), 1e-3);
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(lin::Vector3f({0.0018f, 0.0f, 0.999f}), tf.pointer_vec2_desired_fp->get(), 1e-3);

    std::cout << tf.m_body_cmd_fp->get()[0] << " " << tf.m_body_cmd_fp->get()[1] << " " << tf.m_body_cmd_fp->get()[2];
    std::cout << tf.t_body_cmd_fp->get()[0] << " " << tf.t_body_cmd_fp->get()[1] << " " << tf.t_body_cmd_fp->get()[2];

    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,-adcs::mtr::max_moment}),tf.m_body_cmd_fp->get(), 1e-7);
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({-0.0225f,0.00224974f,-0.00148672f}),tf.t_body_cmd_fp->get(), 1e-7);

    // now change state to manual, and check that the pointing information persisted
    tf.adcs_state_fp->set(static_cast<unsigned char>(adcs_state_t::point_manual));
    tf.step();

    // check that these values have persisted
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(lin::Vector3f({1.0f, 0.0f, 0.0f}), tf.pointer_vec1_current_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(lin::Vector3f({0.0f, 0.0f, 1.0f}), tf.pointer_vec2_current_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(lin::Vector3f({-0.105f, -0.994f, 0.00017f}), tf.pointer_vec1_desired_fp->get(), 1e-3);
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(lin::Vector3f({0.0018f, 0.0f, 0.999f}), tf.pointer_vec2_desired_fp->get(), 1e-3);

    // check these doomed to pass actuator outputs
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,-adcs::mtr::max_moment}),tf.m_body_cmd_fp->get(), 1e-7);
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({-0.0225f,0.00224974f,-0.00148672f}),tf.t_body_cmd_fp->get(), 1e-7);

    // go back to standby
    tf.adcs_state_fp->set(static_cast<unsigned char>(adcs_state_t::point_standby));
    tf.step();

    // lose every signal needed for standby and show that it goes to 0 actuators
    nan_sensors(tf);
    tf.step();
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,0}), tf.t_body_cmd_fp->get(), 1e-10);    
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,0}), tf.m_body_cmd_fp->get(), 1e-10);
}

void test_point_docking() {
    TestFixture tf;
    tf.adcs_state_fp->set(static_cast<unsigned char>(adcs_state_t::point_docking));
    load_good_data(tf);

    tf.step();

    std::cout << tf.pointer_vec1_desired_fp->get();
    std::cout << tf.pointer_vec2_desired_fp->get();

    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(lin::Vector3f({0.0f, 0.0f, 1.0f}), tf.pointer_vec1_current_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(lin::Vector3f({1.0f, 0.0f, 0.0f}), tf.pointer_vec2_current_fp->get(), 1e-10);
    
    // this test is doomed to pass, but the important part is that it is not nan
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(lin::Vector3f({-0.994661f, .103185f, 0.00182815f}), tf.pointer_vec1_desired_fp->get(), 1e-3);
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(lin::Vector3f({0.00183598f, 0.000019023f, 0.999998f}), tf.pointer_vec2_desired_fp->get(), 1e-3);

    std::cout << tf.m_body_cmd_fp->get()[0] << " " << tf.m_body_cmd_fp->get()[1] << " " << tf.m_body_cmd_fp->get()[2];
    std::cout << tf.t_body_cmd_fp->get()[0] << " " << tf.t_body_cmd_fp->get()[1] << " " << tf.t_body_cmd_fp->get()[2];

    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,-adcs::mtr::max_moment}),tf.m_body_cmd_fp->get(), 1e-7);
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({-0.0226458f,0.000844621,-0.0f}),tf.t_body_cmd_fp->get(), 1e-7);

    // lose every signal needed for docking and show that it goes to 0 actuators
    nan_sensors(tf);
    tf.step();
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,0}), tf.t_body_cmd_fp->get(), 1e-10);    
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,0}), tf.m_body_cmd_fp->get(), 1e-10);
}

// the objective of this test is to observe that we can set the pointing objectives to nan
// in order to allow custom actuator commands
void test_point_manual_nan_pointing(){
    TestFixture tf;
    tf.adcs_state_fp->set(static_cast<unsigned char>(adcs_state_t::point_manual));
    load_good_data(tf);

    f_vector_t rand_act{0.01,0.02,-0.03};
    tf.m_body_cmd_fp->set(rand_act);
    tf.t_body_cmd_fp->set(rand_act);

    tf.pointer_vec1_current_fp->set(lin::nans<lin::Vector3f>()); 
    tf.pointer_vec2_current_fp->set(lin::nans<lin::Vector3f>());
    tf.pointer_vec1_desired_fp->set(lin::nans<lin::Vector3f>());
    tf.pointer_vec2_desired_fp->set(lin::nans<lin::Vector3f>());

    tf.step();

    // check these doomed to pass actuator outputs
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(rand_act,tf.m_body_cmd_fp->get(), 1e-7);
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(rand_act,tf.t_body_cmd_fp->get(), 1e-7);   

    // step again to check persistence
    tf.step();

    // check these doomed to pass actuator outputs
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(rand_act,tf.m_body_cmd_fp->get(), 1e-7);
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(rand_act,tf.t_body_cmd_fp->get(), 1e-7);  
}

void test_point_limited() {
    // TODO implement
}

int test_attitude_controller() {
    UNITY_BEGIN();
    RUN_TEST(test_valid_initialization);
    RUN_TEST(test_detumble);
    RUN_TEST(test_standby);
    RUN_TEST(test_point_docking);
    RUN_TEST(test_point_manual_nan_pointing);

    return UNITY_END();
}

#ifdef DESKTOP
int main() {
    return test_attitude_controller();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_attitude_controller();
}

void loop() {}
#endif
