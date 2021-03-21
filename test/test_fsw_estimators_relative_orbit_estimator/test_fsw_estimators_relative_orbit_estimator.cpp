#include "../custom_assertions.hpp"
#include "../StateFieldRegistryMock.hpp"

#include <gnc/constants.hpp>

#include <fsw/FCCode/constants.hpp>
#include <fsw/FCCode/Estimators/OrbitEstimator.hpp>
#include <fsw/FCCode/Estimators/RelativeOrbitEstimator.hpp>
#include <fsw/FCCode/Estimators/rel_orbit_state_t.enum>
#include <fsw/FCCode/Estimators/TimeEstimator.hpp>
#include <fsw/FCCode/piksi_mode_t.enum>

#include <lin/core.hpp>

class TestFixture
{
  public:
    StateFieldRegistryMock registry;

    std::shared_ptr<ReadableStateField<unsigned char>> piksi_mode_fp;
    std::shared_ptr<ReadableStateField<unsigned int>> piksi_microdelta_fp;
    std::shared_ptr<ReadableStateField<gps_time_t>> piksi_time_fp;
    std::shared_ptr<ReadableStateField<lin::Vector3d>> piksi_pos_fp;
    std::shared_ptr<ReadableStateField<lin::Vector3d>> piksi_vel_fp;
    std::shared_ptr<ReadableStateField<lin::Vector3d>> piksi_baseline_pos_fp;

    TimeEstimator time_estimator;

    ReadableStateField<bool> const *const time_valid_fp;
    ReadableStateField<gps_time_t> const *const time_gps_fp;
    InternalStateField<unsigned long long> const *const time_ns_fp;
    InternalStateField<double> const *const time_s_fp;
    WritableStateField<bool> *const time_reset_cmd_fp;

    OrbitEstimator orbit_estimator;

    ReadableStateField<bool> const *const orbit_valid_fp;
    ReadableStateField<lin::Vector3d> const *const orbit_pos_fp;
    ReadableStateField<lin::Vector3d> const *const orbit_vel_fp;
    WritableStateField<bool> *const orbit_reset_cmd_fp;

    RelativeOrbitEstimator relative_orbit_estimator;

    ReadableStateField<unsigned char> const *const rel_orbit_state_fp;
    ReadableStateField<lin::Vector3d> const *const rel_orbit_pos_fp;
    ReadableStateField<lin::Vector3d> const *const rel_orbit_vel_fp;
    ReadableStateField<lin::Vector3d> const *const rel_orbit_rel_pos_fp;
    ReadableStateField<lin::Vector3d> const *const rel_orbit_rel_pos_sigma_fp;
    ReadableStateField<lin::Vector3d> const *const rel_orbit_rel_vel_fp;
    ReadableStateField<lin::Vector3d> const *const rel_orbit_rel_vel_sigma_fp;
    WritableStateField<bool> *const rel_orbit_reset_cmd_fp;
    WritableStateField<gps_time_t> *const rel_orbit_uplink_time_fp;
    WritableStateField<lin::Vector3d> *const rel_orbit_uplink_pos_fp;
    WritableStateField<lin::Vector3d> *const rel_orbit_uplink_vel_fp;

    TestFixture()
        : registry(),
          piksi_mode_fp(registry.create_readable_field<unsigned char>("piksi.state")),
          piksi_microdelta_fp(registry.create_readable_field<unsigned int>("piksi.microdelta")),
          piksi_time_fp(registry.create_readable_field<gps_time_t>("piksi.time")),
          piksi_pos_fp(registry.create_readable_lin_vector_field<double>("piksi.pos", 6771000, 6921000, 28)),
          piksi_vel_fp(registry.create_readable_lin_vector_field<double>("piksi.vel", 7570, 7685, 19)),
          piksi_baseline_pos_fp(registry.create_readable_lin_vector_field<double>("piksi.baseline_pos", 0, 2000, 22)),
          time_estimator(registry),
          time_valid_fp(registry.find_readable_field_t<bool>("time.valid")),
          time_gps_fp(registry.find_readable_field_t<gps_time_t>("time.gps")),
          time_ns_fp(registry.find_internal_field_t<unsigned long long>("time.ns")),
          time_s_fp(registry.find_internal_field_t<double>("time.s")),
          time_reset_cmd_fp(registry.find_writable_field_t<bool>("time.reset_cmd")),
          orbit_estimator(registry),
          orbit_valid_fp(registry.find_readable_field_t<bool>("orbit.valid")),
          orbit_pos_fp(registry.find_readable_field_t<lin::Vector3d>("orbit.pos")),
          orbit_vel_fp(registry.find_readable_field_t<lin::Vector3d>("orbit.vel")),
          orbit_reset_cmd_fp(registry.find_writable_field_t<bool>("orbit.reset_cmd")),
          relative_orbit_estimator(registry),
          rel_orbit_state_fp(registry.find_readable_field_t<unsigned char>("rel_orbit.state")),
          rel_orbit_pos_fp(registry.find_readable_field_t<lin::Vector3d>("rel_orbit.pos")),
          rel_orbit_vel_fp(registry.find_readable_field_t<lin::Vector3d>("rel_orbit.vel")),
          rel_orbit_rel_pos_fp(registry.find_readable_field_t<lin::Vector3d>("rel_orbit.rel_pos")),
          rel_orbit_rel_pos_sigma_fp(registry.find_readable_field_t<lin::Vector3d>("rel_orbit.rel_pos_sigma")),
          rel_orbit_rel_vel_fp(registry.find_readable_field_t<lin::Vector3d>("rel_orbit.rel_vel")),
          rel_orbit_rel_vel_sigma_fp(registry.find_readable_field_t<lin::Vector3d>("rel_orbit.rel_vel_sigma")),
          rel_orbit_reset_cmd_fp(registry.find_writable_field_t<bool>("rel_orbit.reset_cmd")),
          rel_orbit_uplink_time_fp(registry.find_writable_field_t<gps_time_t>("rel_orbit.uplink.time")),
          rel_orbit_uplink_pos_fp(registry.find_writable_field_t<lin::Vector3d>("rel_orbit.uplink.pos")),
          rel_orbit_uplink_vel_fp(registry.find_writable_field_t<lin::Vector3d>("rel_orbit.uplink.vel"))
    { }
};

static auto const pan_epoch = gps_time_t(gnc::constant::init_gps_week_number,
            gnc::constant::init_gps_time_of_week, gnc::constant::init_gps_nanoseconds);
static constexpr lin::Vector3d r_ecef = {6.8538e6, 0.0, 0.0};
static constexpr lin::Vector3d v_ecef = {0.0, 5.3952e3, 5.3952e3};

void test_constructor()
{
    TestFixture tf;

    // By default, the valid flag and reset command should be false
    TEST_ASSERT_FALSE(tf.rel_orbit_state_fp->get());
    TEST_ASSERT_FALSE(tf.rel_orbit_reset_cmd_fp->get());
}

int test_control_task()
{
    UNITY_BEGIN();
    RUN_TEST(test_constructor);
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