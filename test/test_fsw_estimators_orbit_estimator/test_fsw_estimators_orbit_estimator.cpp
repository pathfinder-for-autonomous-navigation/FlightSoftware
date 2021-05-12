#include "../custom_assertions.hpp"
#include "../StateFieldRegistryMock.hpp"

#include <gnc/constants.hpp>

#include <fsw/FCCode/constants.hpp>
#include <fsw/FCCode/Estimators/OrbitEstimator.hpp>
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

    TestFixture()
        : registry(),
          piksi_mode_fp(registry.create_readable_field<unsigned char>("piksi.state")),
          piksi_microdelta_fp(registry.create_readable_field<unsigned int>("piksi.microdelta")),
          piksi_time_fp(registry.create_readable_field<gps_time_t>("piksi.time")),
          piksi_pos_fp(registry.create_readable_lin_vector_field<double>("piksi.pos", 6771000, 6921000, 28)),
          piksi_vel_fp(registry.create_readable_lin_vector_field<double>("piksi.vel", 7570, 7685, 19)),
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
          orbit_reset_cmd_fp(registry.find_writable_field_t<bool>("orbit.reset_cmd"))
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
    TEST_ASSERT_FALSE(tf.orbit_valid_fp->get());
    TEST_ASSERT_FALSE(tf.orbit_reset_cmd_fp->get());
}

void test_update()
{
    // Ensure the estimate doesn't update for invalid Piksi states
    auto assert_no_update_for = [](piksi_mode_t mode) {
        TestFixture tf;

        // Initialize a time estimate
        tf.piksi_mode_fp->set(static_cast<unsigned char>(piksi_mode_t::spp));
        tf.piksi_microdelta_fp->set(0);
        tf.piksi_time_fp->set(pan_epoch);

        tf.time_estimator.execute();

        TEST_ASSERT_TRUE(tf.time_valid_fp->get());

        // Ensure now the orbit estimator won't update in the given Piksi mode
        tf.piksi_mode_fp->set(static_cast<unsigned char>(mode));
        tf.piksi_pos_fp->set(r_ecef);
        tf.piksi_vel_fp->set(v_ecef);

        tf.time_estimator.execute();
        tf.orbit_estimator.execute();

        TEST_ASSERT_FALSE(tf.orbit_valid_fp->get());
        TEST_ASSERT_TRUE(tf.time_valid_fp->get());
    };
    assert_no_update_for(piksi_mode_t::no_fix);
    assert_no_update_for(piksi_mode_t::sync_error);
    assert_no_update_for(piksi_mode_t::nsat_error);
    assert_no_update_for(piksi_mode_t::crc_error);
    assert_no_update_for(piksi_mode_t::time_limit_error);
    assert_no_update_for(piksi_mode_t::data_error);
    assert_no_update_for(piksi_mode_t::no_data_error);

    // Ensure the estimate does update for valid Piksi states
    auto assert_update_for = [](piksi_mode_t mode) {
        TestFixture tf;

        // Initialize a time estimate
        tf.piksi_mode_fp->set(static_cast<unsigned char>(piksi_mode_t::spp));
        tf.piksi_microdelta_fp->set(0);
        tf.piksi_time_fp->set(pan_epoch);

        tf.time_estimator.execute();

        TEST_ASSERT_TRUE(tf.time_valid_fp->get());

        // Ensure now the orbit estimator will update in the given Piksi mode
        tf.piksi_mode_fp->set(static_cast<unsigned char>(mode));
        tf.piksi_pos_fp->set(r_ecef);
        tf.piksi_vel_fp->set(v_ecef);

        tf.time_estimator.execute();
        tf.orbit_estimator.execute();

        TEST_ASSERT_TRUE(tf.orbit_valid_fp->get());
        TEST_ASSERT_TRUE(tf.time_valid_fp->get());
        TEST_ASSERT_DOUBLE_WITHIN(1.0e-3, lin::fro(tf.orbit_pos_fp->get() - r_ecef), 0.0);
        TEST_ASSERT_DOUBLE_WITHIN(1.0e-3, lin::fro(tf.orbit_vel_fp->get() - v_ecef), 0.0);
    };
    assert_update_for(piksi_mode_t::spp);
    assert_update_for(piksi_mode_t::fixed_rtk);
    assert_update_for(piksi_mode_t::float_rtk);
}

void test_reset()
{
    TestFixture tf;

    // Initialize the time and orbit estimates
    tf.piksi_mode_fp->set(static_cast<unsigned char>(piksi_mode_t::spp));
    tf.piksi_microdelta_fp->set(0);
    tf.piksi_time_fp->set(pan_epoch);
    tf.piksi_pos_fp->set(r_ecef);
    tf.piksi_vel_fp->set(v_ecef);

    tf.time_estimator.execute();
    tf.orbit_estimator.execute();

    TEST_ASSERT_TRUE(tf.time_valid_fp->get());
    TEST_ASSERT_TRUE(tf.orbit_valid_fp->get());

    // Reseting the time estimator should reset the orbit estimate
    tf.time_reset_cmd_fp->set(true);

    tf.time_estimator.execute();
    tf.orbit_estimator.execute();

    TEST_ASSERT_FALSE(tf.time_valid_fp->get());
    TEST_ASSERT_FALSE(tf.time_reset_cmd_fp->get());
    TEST_ASSERT_FALSE(tf.orbit_valid_fp->get());
    TEST_ASSERT_FALSE(tf.orbit_reset_cmd_fp->get());

    // Everything should reset on the next control cycle
    tf.time_estimator.execute();
    tf.orbit_estimator.execute();

    TEST_ASSERT_TRUE(tf.time_valid_fp->get());
    TEST_ASSERT_TRUE(tf.orbit_valid_fp->get());

    // Just reset the orbit estimate
    tf.orbit_reset_cmd_fp->set(true);

    tf.time_estimator.execute();
    tf.orbit_estimator.execute();

    TEST_ASSERT_TRUE(tf.time_valid_fp->get());
    TEST_ASSERT_FALSE(tf.time_reset_cmd_fp->get());
    TEST_ASSERT_FALSE(tf.orbit_valid_fp->get());
    TEST_ASSERT_FALSE(tf.orbit_reset_cmd_fp->get());
}

int test_control_task()
{
    UNITY_BEGIN();
    RUN_TEST(test_constructor);
    RUN_TEST(test_update);
    RUN_TEST(test_reset);
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
