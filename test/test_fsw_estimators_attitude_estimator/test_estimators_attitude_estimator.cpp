#include "../custom_assertions.hpp"
#include "../StateFieldRegistryMock.hpp"

#include <gnc/constants.hpp>

#include <fsw/FCCode/Estimators/AttitudeEstimator.hpp>
#include <fsw/FCCode/Estimators/OrbitEstimator.hpp>
#include <fsw/FCCode/Estimators/TimeEstimator.hpp>
#include <fsw/FCCode/piksi_mode_t.enum>

#include <lin/core.hpp>

#include "../custom_assertions.hpp"

class TestFixture {
  public:
    StateFieldRegistryMock registry;

    std::shared_ptr<ReadableStateField<unsigned char>> piksi_mode_fp;
    std::shared_ptr<ReadableStateField<unsigned int>> piksi_microdelta_fp;
    std::shared_ptr<ReadableStateField<gps_time_t>> piksi_time_fp;
    std::shared_ptr<ReadableStateField<lin::Vector3d>> piksi_pos_fp;
    std::shared_ptr<ReadableStateField<lin::Vector3d>> piksi_vel_fp;

    TimeEstimator time_estimator;

    ReadableStateField<bool> const *const time_valid_fp;

    OrbitEstimator orbit_estimator;

    ReadableStateField<bool> const *const orbit_valid_fp;

    std::shared_ptr<ReadableStateField<bool>> adcs_gyr_functional_fp;
    std::shared_ptr<ReadableStateField<lin::Vector3f>> adcs_gyr_fp;
    std::shared_ptr<ReadableStateField<bool>> adcs_mag1_functional_fp;
    std::shared_ptr<ReadableStateField<lin::Vector3f>> adcs_mag1_fp;
    std::shared_ptr<ReadableStateField<bool>> adcs_mag2_functional_fp;
    std::shared_ptr<ReadableStateField<lin::Vector3f>> adcs_mag2_fp;
    std::shared_ptr<ReadableStateField<unsigned char>> adcs_ssa_mode_fp;
    std::shared_ptr<ReadableStateField<lin::Vector3f>> adcs_ssa_fp;

    AttitudeEstimator attitude_estimator;

    ReadableStateField<bool> const *const attitude_estimator_b_valid_fp;
    ReadableStateField<bool> const *const attitude_estimator_valid_fp;
    WritableStateField<bool> const *const attitude_estimator_reset_cmd_fp;

    TestFixture()
        : registry(),
          piksi_mode_fp(registry.create_readable_field<unsigned char>("piksi.state")),
          piksi_microdelta_fp(registry.create_readable_field<unsigned int>("piksi.microdelta")),
          piksi_time_fp(registry.create_readable_field<gps_time_t>("piksi.time")),
          piksi_pos_fp(registry.create_readable_lin_vector_field<double>("piksi.pos", 6771000, 6921000, 28)),
          piksi_vel_fp(registry.create_readable_lin_vector_field<double>("piksi.vel", 7570, 7685, 19)),
          time_estimator(registry),
          time_valid_fp(registry.find_readable_field_t<bool>("time.valid")),
          orbit_estimator(registry),
          orbit_valid_fp(registry.find_readable_field_t<bool>("orbit.valid")),
          adcs_gyr_functional_fp(registry.create_readable_field<bool>("adcs_monitor.havt_device0")),
          adcs_gyr_fp(registry.create_readable_lin_vector_field<float>("adcs_monitor.gyr_vec",0, 1, 1)),
          adcs_mag1_functional_fp(registry.create_readable_field<bool>("adcs_monitor.havt_device1")),
          adcs_mag1_fp(registry.create_readable_lin_vector_field<float>("adcs_monitor.mag1_vec", 0.0, 16e-4, 1)),
          adcs_mag2_functional_fp(registry.create_readable_field<bool>("adcs_monitor.havt_device2")),
          adcs_mag2_fp(registry.create_readable_lin_vector_field<float>("adcs_monitor.mag2_vec", 0.0, 16e-4, 1)),
          adcs_ssa_mode_fp(registry.create_readable_field<unsigned char>("adcs_monitor.ssa_mode", 3)),
          adcs_ssa_fp(registry.create_readable_lin_vector_field<float>("adcs_monitor.ssa_vec", 0, 1, 1)),
          attitude_estimator(registry),
          attitude_estimator_b_valid_fp(registry.find_readable_field_t<bool>("attitude_estimator.b_valid")),
          attitude_estimator_valid_fp(registry.find_readable_field_t<bool>("attitude_estimator.valid")),
          attitude_estimator_reset_cmd_fp(registry.find_writable_field_t<bool>("attitude_estimator.reset_cmd"))
    { }
};

void test_task_initialization()
{
    TestFixture tf;

    TEST_ASSERT_FALSE(tf.attitude_estimator_b_valid_fp->get());
    TEST_ASSERT_FALSE(tf.attitude_estimator_valid_fp->get());
    TEST_ASSERT_FALSE(tf.attitude_estimator_reset_cmd_fp->get());
}

int test_control_task()
{
        UNITY_BEGIN();
        RUN_TEST(test_task_initialization);
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

void loop()
{ }
#endif
