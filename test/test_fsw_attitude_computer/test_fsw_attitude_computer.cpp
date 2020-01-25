#include "../StateFieldRegistryMock.hpp"
#include "../custom_assertions.hpp"

#include <fsw/FCCode/AttitudeComputer.hpp>
#include <fsw/FCCode/adcs_state_t.enum>

#include <unity.h>
#include <gnc_constants.hpp>

constexpr float nan_f = std::numeric_limits<float>::quiet_NaN();

class TestFixture {
  public:
    StateFieldRegistryMock registry;
    // Input state fields to attitude computer
    std::shared_ptr<WritableStateField<unsigned char>> adcs_state_fp;
    std::shared_ptr<ReadableStateField<f_quat_t>> q_body_eci_fp;
    std::shared_ptr<ReadableStateField<f_vector_t>> ssa_vec_fp;
    std::shared_ptr<ReadableStateField<d_vector_t>> pos_fp;
    std::shared_ptr<ReadableStateField<d_vector_t>> pos_baseline_fp;

    // Attitude computer
    std::unique_ptr<AttitudeComputer> attitude_computer;
    
    // Output state fields
    WritableStateField<f_vector_t>* adcs_vec1_current_fp;
    WritableStateField<f_vector_t>* adcs_vec1_desired_fp;
    WritableStateField<f_vector_t>* adcs_vec2_current_fp;
    WritableStateField<f_vector_t>* adcs_vec2_desired_fp;

    TestFixture() : registry() {
        adcs_state_fp = registry.create_writable_field<unsigned char>("adcs.state", 8);
        q_body_eci_fp = registry.create_readable_field<f_quat_t>("attitude_estimator.q_body_eci");
        ssa_vec_fp = registry.create_readable_vector_field<float>("adcs_monitor.ssa_vec", 0, 1, 100);
        pos_fp = registry.create_readable_vector_field<double>("orbit.pos", 0, 100000, 100);
        pos_baseline_fp = registry.create_readable_vector_field<double>("orbit.baseline_pos", 0, 100000, 100);

        attitude_computer = std::make_unique<AttitudeComputer>(registry, 0);

        // Check that attitude computer creates its expected fields
        adcs_vec1_current_fp = registry.find_writable_field_t<f_vector_t>("adcs.control.vec1.current");
        adcs_vec1_desired_fp = registry.find_writable_field_t<f_vector_t>("adcs.control.vec1.desired");
        adcs_vec2_current_fp = registry.find_writable_field_t<f_vector_t>("adcs.control.vec2.current");
        adcs_vec2_desired_fp = registry.find_writable_field_t<f_vector_t>("adcs.control.vec2.desired");

        // Set quaternion to non-rotating value
        q_body_eci_fp->set({0,0,0,1});
    }
};

void test_valid_initialization() {
    TestFixture tf;

    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({nan_f, nan_f, nan_f}).data(), tf.adcs_vec1_current_fp->get().data(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({nan_f, nan_f, nan_f}).data(), tf.adcs_vec2_current_fp->get().data(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({nan_f, nan_f, nan_f}).data(), tf.adcs_vec1_desired_fp->get().data(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({nan_f, nan_f, nan_f}).data(), tf.adcs_vec2_desired_fp->get().data(), 1e-10);
}

void test_point_standby() {
    TestFixture tf;
    
    // Test pointing without GPS
    tf.adcs_state_fp->set(static_cast<unsigned char>(adcs_state_t::point_standby));
    tf.pos_fp->set({nan_f, nan_f, nan_f});
    tf.ssa_vec_fp->set({sqrtf(2)/2,sqrtf(2)/2,0});
    tf.attitude_computer->execute();
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({sqrtf(2)/2,sqrtf(2)/2,0}).data(), tf.adcs_vec1_current_fp->get().data(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({sqrtf(2)/2,sqrtf(2)/2,0}).data(), tf.adcs_vec1_desired_fp->get().data(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({nan_f,nan_f,nan_f}).data(), tf.adcs_vec2_current_fp->get().data(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({nan_f,nan_f,nan_f}).data(), tf.adcs_vec2_desired_fp->get().data(), 1e-10);

    // Test long edge choice calculation for several choices of long edge
    // Choice 1
    tf.ssa_vec_fp->set({sqrtf(2)/2,-sqrtf(2)/2,0});
    tf.attitude_computer->execute();
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({sqrtf(2)/2,-sqrtf(2)/2,0}).data(), tf.adcs_vec1_desired_fp->get().data(), 1e-10);
    // Choice 2
    tf.ssa_vec_fp->set({-sqrtf(2)/2,sqrtf(2)/2,0});
    tf.attitude_computer->execute();
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({-sqrtf(2)/2,sqrtf(2)/2,0}).data(), tf.adcs_vec1_desired_fp->get().data(), 1e-10);
    // Choice 3
    tf.ssa_vec_fp->set({-sqrtf(2)/2,-sqrtf(2)/2,0});
    tf.attitude_computer->execute();
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({-sqrtf(2)/2,-sqrtf(2)/2,0}).data(), tf.adcs_vec1_desired_fp->get().data(), 1e-10);

    // Test pointing with GPS data: with and without a secondary pointing objective
    // With secondary pointing objective
    tf.ssa_vec_fp->set({1,0,0});
    tf.pos_fp->set({0,2,0});
    tf.attitude_computer->execute();
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,1,0}).data(), tf.adcs_vec1_current_fp->get().data(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({1,0,0}).data(), tf.adcs_vec1_desired_fp->get().data(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,-1}).data(), tf.adcs_vec2_current_fp->get().data(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,1}).data(), tf.adcs_vec2_desired_fp->get().data(), 1e-10);
    // Without secondary pointing objective
    tf.pos_fp->set({0,2,0});
    tf.ssa_vec_fp->set({0,2 - 1e-5,0});
    tf.attitude_computer->execute();
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,1}).data(), tf.adcs_vec2_current_fp->get().data(), 1e-10);
}

void test_point_docking() {
    TestFixture tf;

    tf.adcs_state_fp->set(static_cast<unsigned char>(adcs_state_t::point_docking));
    tf.pos_fp->set({0,2,0});
    tf.pos_baseline_fp->set({0,0,3});
    tf.attitude_computer->execute();

    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,1,0}).data(), tf.adcs_vec1_current_fp->get().data(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({1,0,0}).data(), tf.adcs_vec1_desired_fp->get().data(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,1}).data(), tf.adcs_vec2_current_fp->get().data(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,1}).data(), tf.adcs_vec2_desired_fp->get().data(), 1e-10);
}

void test_point_limited() {
    // TODO implement
}

// If the two desired vectors end up being within 10 degrees of each other, the
// attitude computer should cancel the secondary pointing objective.
void test_parallel_objectives() {
    // Secondary pointing objective should be canceled if the second current vector is NaN
    {
        TestFixture tf;
        tf.pos_fp->set({0,0,0}); // Set pos to be non-NaN so that vec2_desired cannot be NaN autonomously
        tf.adcs_vec2_current_fp->set({nan_f, nan_f, nan_f}); // 8 degrees away
        tf.attitude_computer->execute();
        PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({nan_f, nan_f, nan_f}),
            tf.adcs_vec2_desired_fp->get().data(), 1e-10);
    }

    // Secondary pointing objective should be canceled if the second desired vector is NaN
    {
        TestFixture tf;
        tf.pos_fp->set({0,0,0});
        tf.adcs_vec2_desired_fp->set({nan_f, nan_f, nan_f}); // 8 degrees away
        tf.attitude_computer->execute();
        PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({nan_f, nan_f, nan_f}),
            tf.adcs_vec2_desired_fp->get().data(), 1e-10);
    }

    // Secondary pointing objective should be canceled if the "current" vectors are within 10
    // degrees of each other
    {
        TestFixture tf;
        tf.pos_fp->set({0,0,0});
        tf.adcs_vec1_current_fp->set({1,0,0});
        tf.adcs_vec2_current_fp->set({
            std::cos(8.0f * gnc::constant::pi_f / 180.0f),
            std::sin(8.0f * gnc::constant::pi_f / 180.0f),
            0}); // 8 degrees away
        tf.attitude_computer->execute();
        PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({nan_f, nan_f, nan_f}),
            tf.adcs_vec2_desired_fp->get().data(), 1e-10);
    }

    // Secondary pointing objective should be canceled if the desired vectors are within 10
    // degrees of each other
    {
        TestFixture tf;
        tf.pos_fp->set({0,0,0});
        tf.adcs_vec1_desired_fp->set({1,0,0});
        tf.adcs_vec2_desired_fp->set({
            std::cos(8.0f * gnc::constant::pi_f / 180.0f),
            std::sin(8.0f * gnc::constant::pi_f / 180.0f),
            0}); // 8 degrees away
        tf.attitude_computer->execute();
        PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({nan_f, nan_f, nan_f}),
            tf.adcs_vec2_desired_fp->get().data(), 1e-10);
    }
}

int test_attitude_computer() {
    UNITY_BEGIN();
    RUN_TEST(test_valid_initialization);
    RUN_TEST(test_point_standby);
    RUN_TEST(test_point_docking);
    RUN_TEST(test_point_limited);
    // We cannot test point_manual since there's nothing happening inside that state.
    RUN_TEST(test_parallel_objectives);
    return UNITY_END();
}

#ifdef DESKTOP
int main() {
    return test_attitude_computer();
}
#else
#include <Arduino.h>
void setup() {
    delay(2000);
    Serial.begin(9600);
    test_attitude_computer();
}

void loop() {}
#endif
