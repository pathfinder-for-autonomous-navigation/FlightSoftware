#include "../StateFieldRegistryMock.hpp"
#include "../src/FCCode/AttitudeComputer.hpp"
#include "../src/FCCode/adcs_state_t.enum"

#include <unity.h>
#include "../custom_assertions.hpp"

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
    const WritableStateField<f_vector_t>* adcs_vec1_current_fp;
    const WritableStateField<f_vector_t>* adcs_vec1_desired_fp;
    const WritableStateField<f_vector_t>* adcs_vec2_current_fp;
    const WritableStateField<f_vector_t>* adcs_vec2_desired_fp;

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
}

void test_point_standby() {
    TestFixture tf;
    constexpr float nan = std::numeric_limits<float>::quiet_NaN();
    
    // Test pointing without GPS
    tf.adcs_state_fp->set(static_cast<unsigned char>(adcs_state_t::point_standby));
    tf.pos_fp->set({nan, nan, nan});
    tf.ssa_vec_fp->set({sqrtf(2)/2,sqrtf(2)/2,0});
    tf.attitude_computer->execute();
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({sqrtf(2)/2,sqrtf(2)/2,0}).data(), tf.adcs_vec1_current_fp->get().data(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({sqrtf(2)/2,sqrtf(2)/2,0}).data(), tf.adcs_vec1_desired_fp->get().data(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({nan,nan,nan}).data(), tf.adcs_vec2_current_fp->get().data(), 1e-10);
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({nan,nan,nan}).data(), tf.adcs_vec2_desired_fp->get().data(), 1e-10);

    // Test long edge choice calculation
    tf.ssa_vec_fp->set({sqrtf(2)/2,-sqrtf(2)/2,0});
    tf.attitude_computer->execute();
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({sqrtf(2)/2,-sqrtf(2)/2,0}).data(), tf.adcs_vec1_desired_fp->get().data(), 1e-10);
    tf.ssa_vec_fp->set({-sqrtf(2)/2,sqrtf(2)/2,0});
    tf.attitude_computer->execute();
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({-sqrtf(2)/2,sqrtf(2)/2,0}).data(), tf.adcs_vec1_desired_fp->get().data(), 1e-10);
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
    PAN_TEST_ASSERT_EQUAL_FLOAT_VEC(f_vector_t({0,0,-1}).data(), tf.adcs_vec2_desired_fp->get().data(), 1e-10);
}

void test_point_limited() {
    // TODO implement
}

int test_attitude_computer() {
    UNITY_BEGIN();
    RUN_TEST(test_valid_initialization);
    RUN_TEST(test_point_standby);
    RUN_TEST(test_point_docking);
    RUN_TEST(test_point_limited);
    // We cannot test point_manual since there's nothing happening inside that state.
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
