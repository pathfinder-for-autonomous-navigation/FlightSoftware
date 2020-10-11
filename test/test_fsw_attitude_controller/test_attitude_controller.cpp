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
        b_body_rd_fp = registry.create_readable_lin_vector_field<float>("adcs_monitor.mag_vec", 0, 1, 100);
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

void test_valid_initialization() {
    TestFixture tf;

    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(nan_vector, tf.pointer_vec1_current_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(nan_vector, tf.pointer_vec2_current_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(nan_vector, tf.pointer_vec1_desired_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(nan_vector, tf.pointer_vec2_desired_fp->get(), 1e-10);

    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,0}), tf.t_body_cmd_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,0}), tf.m_body_cmd_fp->get(), 1e-10);
}

void test_detumble(){
    TestFixture tf;

    tf.adcs_state_fp->set(static_cast<unsigned char>(adcs_state_t::detumble));
    tf.b_body_est_fp->set(lin::Vector3f({1,-1,0}));
    
    tf.step();
    
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(nan_vector, tf.pointer_vec1_current_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(nan_vector, tf.pointer_vec2_current_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(nan_vector, tf.pointer_vec1_desired_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(nan_vector, tf.pointer_vec2_desired_fp->get(), 1e-10);

    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,0}), tf.t_body_cmd_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,0}), tf.m_body_cmd_fp->get(), 1e-10);

    // dump in 8 data points to almost fill the buffer
    for(int i = 0; i<8; i++){
        tf.b_body_est_fp->set(lin::Vector3f({1,-1,0}));
        tf.step();
    }

    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,0}), tf.m_body_cmd_fp->get(), 1e-10);

    // last data point, now size = 10 and we should have a non zero mtr actuation
    tf.b_body_est_fp->set(lin::Vector3f({-1,1,0}));
    tf.step();

    f_vector_t local_m = tf.m_body_cmd_fp->get();

    printf("%f,%f,%f", local_m[0],local_m[1],local_m[2]);

    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t(
        {adcs::mtr::max_moment,-adcs::mtr::max_moment,0}), tf.m_body_cmd_fp->get(), 1e-10);

}

void test_standby(){
    TestFixture tf;

}

void test_point_standby_old() {
    TestFixture tf;
    
    // Test pointing without GPS
    tf.adcs_state_fp->set(static_cast<unsigned char>(adcs_state_t::point_standby));
    tf.pos_ecef_fp->set(lin::Vector3f({nan_f, nan_f, nan_f}));
    tf.s_body_est_fp->set(lin::Vector3f({std::sqrt(2.0f)/2.0f,std::sqrt(2.0f)/2.0f,0.0f}));
    tf.attitude_controller->execute();
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(lin::Vector3f({std::sqrt(2.0f)/2.0f,std::sqrt(2.0f)/2.0f,0.0f}), tf.pointer_vec1_current_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(lin::Vector3f({std::sqrt(2.0f)/2.0f,std::sqrt(2.0f)/2.0f,0.0f}), tf.pointer_vec1_desired_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(lin::Vector3f({nan_f,nan_f,nan_f}), tf.pointer_vec2_current_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(lin::Vector3f({nan_f,nan_f,nan_f}), tf.pointer_vec2_desired_fp->get(), 1e-10);

    // Test long edge choice calculation for several choices of long edge
    // Choice 1
    tf.s_body_est_fp->set(lin::Vector3f({std::sqrt(2.0f)/2.0f,-std::sqrt(2.0f)/2.0f,0.0f}));
    tf.attitude_controller->execute();
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(lin::Vector3f({std::sqrt(2.0f)/2.0f,-std::sqrt(2.0f)/2.0f,0.0f}), tf.pointer_vec1_desired_fp->get(), 1e-10);
    // Choice 2
    tf.s_body_est_fp->set(lin::Vector3f({-std::sqrt(2.0f)/2.0f,std::sqrt(2.0f)/2.0f,0.0f}));
    tf.attitude_controller->execute();
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(lin::Vector3f({-std::sqrt(2.0f)/2.0f,std::sqrt(2.0f)/2.0f,0.0f}), tf.pointer_vec1_desired_fp->get(), 1e-10);
    // Choice 3
    tf.s_body_est_fp->set(lin::Vector3f({-std::sqrt(2.0f)/2.0f,-std::sqrt(2.0f)/2.0f,0.0f}));
    tf.attitude_controller->execute();
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(lin::Vector3f({-std::sqrt(2.0f)/2.0f,-std::sqrt(2.0f)/2.0f,0.0f}), tf.pointer_vec1_desired_fp->get(), 1e-10);

    // Test pointing with GPS data: with and without a secondary pointing objective
    // With secondary pointing objective
    tf.s_body_est_fp->set(lin::Vector3f({1.0f,0.0f,0.0f}));
    tf.pos_ecef_fp->set(lin::Vector3f({0.0f,2.0f,0.0f}));
    tf.attitude_controller->execute();
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(lin::Vector3f({0,1,0}), tf.pointer_vec1_current_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(lin::Vector3f({1,0,0}), tf.pointer_vec1_desired_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(lin::Vector3f({0,0,-1}), tf.pointer_vec2_current_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(lin::Vector3f({0,0,1}), tf.pointer_vec2_desired_fp->get(), 1e-10);
    // Without secondary pointing objective
    tf.pos_ecef_fp->set(lin::Vector3f({0.0f,2.0f,0.0f}));
    tf.s_body_est_fp->set(lin::Vector3f({0.0f,1.99999f,0.0f}));
    tf.attitude_controller->execute();
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(lin::Vector3f({0,0,1}), tf.pointer_vec2_current_fp->get(), 1e-10);
}

void test_point_docking() {
    TestFixture tf;

    tf.adcs_state_fp->set(static_cast<unsigned char>(adcs_state_t::point_docking));
    tf.pos_ecef_fp->set(lin::Vector3f({0.0f,2.0f,0.0f}));
    tf.pos_baseline_ecef_fp->set(lin::Vector3f({0.0f,0.0f,3.0f}));
    tf.attitude_controller->execute();

    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(lin::Vector3f({0.0f,1.0f,0.0f}), tf.pointer_vec1_current_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(lin::Vector3f({1.0f,0.0f,0.0f}), tf.pointer_vec1_desired_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(lin::Vector3f({0.0f,0.0f,1.0f}), tf.pointer_vec2_current_fp->get(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(lin::Vector3f({0.0f,0.0f,1.0f}), tf.pointer_vec2_desired_fp->get(), 1e-10);
}

void test_point_limited() {
    // TODO implement
}

// If the two desired vectors end up being within 10 degrees of each other, the
// attitude controller should cancel the secondary pointing objective.
void test_parallel_objectives() {
    // Secondary pointing objective should be canceled if the second current vector is NaN
    {
        TestFixture tf;
        tf.pos_ecef_fp->set(lin::Vector3f({0.0f,0.0f,0.0f})); // Set pos to be non-NaN so that vec2_desired cannot be NaN autonomously
        tf.pointer_vec2_current_fp->set(lin::Vector3f({nan_f, nan_f, nan_f})); // 8 degrees away
        tf.attitude_controller->execute();
        PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(nan_vector,
            tf.pointer_vec2_desired_fp->get(), 1e-10);
    }

    // Secondary pointing objective should be canceled if the second desired vector is NaN
    {
        TestFixture tf;
        tf.pos_ecef_fp->set(lin::Vector3f({0.0f,0.0f,0.0f}));
        tf.pointer_vec2_desired_fp->set(lin::Vector3f({nan_f, nan_f, nan_f})); // 8 degrees away
        tf.attitude_controller->execute();
        PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(nan_vector,
            tf.pointer_vec2_desired_fp->get(), 1e-10);
    }

    // Secondary pointing objective should be canceled if the "current" vectors are within 10
    // degrees of each other
    {
        TestFixture tf;
        tf.pos_ecef_fp->set(lin::Vector3f({0.0f,0.0f,0.0f}));
        tf.pointer_vec1_current_fp->set(lin::Vector3f({1.0f,0.0f,0.0f}));
        tf.pointer_vec2_current_fp->set(lin::Vector3f({
            std::cos(8.0f * gnc::constant::pi_f / 180.0f),
            std::sin(8.0f * gnc::constant::pi_f / 180.0f),
            0.0f})); // 8 degrees away
        tf.attitude_controller->execute();
        PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(nan_vector,
            tf.pointer_vec2_desired_fp->get(), 1e-10);
    }

    // Secondary pointing objective should be canceled if the desired vectors are within 10
    // degrees of each other
    {
        TestFixture tf;
        tf.pos_ecef_fp->set(lin::Vector3f({0.0f,0.0f,0.0f}));
        tf.pointer_vec1_desired_fp->set(lin::Vector3f({1.0f,0.0f,0.0f}));
        tf.pointer_vec2_desired_fp->set(lin::Vector3f({
            std::cos(8.0f * gnc::constant::pi_f / 180.0f),
            std::sin(8.0f * gnc::constant::pi_f / 180.0f),
            0.0f})); // 8 degrees away
        tf.attitude_controller->execute();
        PAN_TEST_ASSERT_EQUAL_FLOAT_LIN_VEC(nan_vector,
            tf.pointer_vec2_desired_fp->get(), 1e-10);
    }
}

int test_attitude_controller() {
    UNITY_BEGIN();
    RUN_TEST(test_valid_initialization);
    RUN_TEST(test_detumble);
    RUN_TEST(test_standby);
    // RUN_TEST(test_point_docking);
    // RUN_TEST(test_point_limited);
    // We cannot test point_manual since there's nothing happening inside that state.
    // RUN_TEST(test_parallel_objectives);
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
