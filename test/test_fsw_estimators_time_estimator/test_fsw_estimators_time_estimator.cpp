#include "../custom_assertions.hpp"
#include "../StateFieldRegistryMock.hpp"

#include <gnc/constants.hpp>

#include <fsw/FCCode/constants.hpp>
#include <fsw/FCCode/Estimators/TimeEstimator.hpp>
#include <fsw/FCCode/piksi_mode_t.enum>

class TestFixture
{
  public:
    StateFieldRegistryMock registry;

    std::shared_ptr<ReadableStateField<unsigned char>> piksi_mode_fp;
    std::shared_ptr<ReadableStateField<unsigned int>> piksi_microdelta_fp;
    std::shared_ptr<ReadableStateField<gps_time_t>> piksi_time_fp;

    TimeEstimator time_estimator;

    ReadableStateField<bool> const *const time_valid_fp;
    ReadableStateField<gps_time_t> const *const time_gps_fp;
    InternalStateField<unsigned long long> const *const time_ns_fp;
    InternalStateField<double> const *const time_s_fp;
    WritableStateField<bool> *const time_reset_cmd_fp;

    TestFixture()
        : registry(),
          piksi_mode_fp(registry.create_readable_field<unsigned char>("piksi.state")),
          piksi_microdelta_fp(registry.create_readable_field<unsigned int>("piksi.microdelta")),
          piksi_time_fp(registry.create_readable_field<gps_time_t>("piksi.time")),
          time_estimator(registry),
          time_valid_fp(registry.find_readable_field_t<bool>("time.valid")),
          time_gps_fp(registry.find_readable_field_t<gps_time_t>("time.gps")),
          time_ns_fp(registry.find_internal_field_t<unsigned long long>("time.ns")),
          time_s_fp(registry.find_internal_field_t<double>("time.s")),
          time_reset_cmd_fp(registry.find_writable_field_t<bool>("time.reset_cmd"))
    { }
};

static auto const pan_epoch = gps_time_t(gnc::constant::init_gps_week_number,
            gnc::constant::init_gps_time_of_week, gnc::constant::init_gps_nanoseconds);

void test_constructor()
{
    TestFixture tf;

    // By default, the valid flag and reset command should be false
    TEST_ASSERT_FALSE(tf.time_valid_fp->get());
    TEST_ASSERT_FALSE(tf.time_reset_cmd_fp->get());
}

void test_update()
{
    TestFixture tf;

    // Ensure the estimate doesn't update for invalid Piksi states
    auto assert_no_update_for = [&](piksi_mode_t mode) {
        tf.piksi_mode_fp->set(static_cast<unsigned char>(mode));
        tf.time_estimator.execute();

        TEST_ASSERT_FALSE(tf.time_valid_fp->get());
    };
    assert_no_update_for(piksi_mode_t::no_fix);
    assert_no_update_for(piksi_mode_t::sync_error);
    assert_no_update_for(piksi_mode_t::nsat_error);
    assert_no_update_for(piksi_mode_t::crc_error);
    assert_no_update_for(piksi_mode_t::time_limit_error);
    assert_no_update_for(piksi_mode_t::data_error);
    assert_no_update_for(piksi_mode_t::no_data_error);

    // Provide an initial time estimate
    tf.piksi_mode_fp->set(static_cast<unsigned char>(piksi_mode_t::spp));
    tf.piksi_microdelta_fp->set(0);
    tf.piksi_time_fp->set(pan_epoch);
    tf.time_estimator.execute();

    TEST_ASSERT_TRUE(tf.time_valid_fp->get());

    // Ensure the estimate updates for all valid Piksi states
    auto assert_update_for = [&](piksi_mode_t mode) {
        tf.piksi_mode_fp->set(static_cast<unsigned char>(mode));
        tf.piksi_time_fp->set(tf.time_gps_fp->get() + PAN::control_cycle_time_ns);
        tf.time_estimator.execute();

        auto const dt_ns = static_cast<unsigned long long>(tf.piksi_time_fp->get() - pan_epoch);
        TEST_ASSERT(tf.time_gps_fp->get() == tf.piksi_time_fp->get());
        TEST_ASSERT_EQUAL(tf.time_ns_fp->get(), dt_ns);
        TEST_ASSERT_EQUAL_DOUBLE(tf.time_s_fp->get(), static_cast<double>(dt_ns) * 1.0e-9);
    };
    assert_update_for(piksi_mode_t::spp);
    assert_update_for(piksi_mode_t::fixed_rtk);
    assert_update_for(piksi_mode_t::float_rtk);

    // Ensure once initialized the time estimate is just incremented
    tf.piksi_mode_fp->set(static_cast<unsigned char>(piksi_mode_t::no_fix));
    for (auto i = 0; i < 100; i++) {
        auto const last_time = tf.time_gps_fp->get();
        tf.time_estimator.execute();

        TEST_ASSERT(tf.time_gps_fp->get() == (last_time + PAN::control_cycle_time_ns));
    }

    // Ensure the time estimate will "jump" back in time
    tf.piksi_mode_fp->set(static_cast<unsigned char>(piksi_mode_t::spp));
    tf.piksi_time_fp->set(pan_epoch);
    tf.time_estimator.execute();

    TEST_ASSERT(tf.time_gps_fp->get() == pan_epoch);
    TEST_ASSERT_EQUAL(tf.time_ns_fp->get(), 0);
    TEST_ASSERT_EQUAL_DOUBLE(tf.time_s_fp->get(), 0.0);
}

void test_microdelta()
{
    static constexpr auto microdelta = 50u;
    static constexpr auto nanodelta = 1000ull * microdelta;

    TestFixture tf;

    // Ensure the microdelta is accounted for in time estimates
    tf.piksi_mode_fp->set(static_cast<unsigned char>(piksi_mode_t::spp));
    tf.piksi_microdelta_fp->set(microdelta);
    tf.piksi_time_fp->set(pan_epoch);
    tf.time_estimator.execute();

    TEST_ASSERT_TRUE(tf.time_valid_fp->get());
    TEST_ASSERT_EQUAL(tf.time_ns_fp->get(), nanodelta);
}

void test_reset()
{
    TestFixture tf;

    tf.piksi_mode_fp->set(static_cast<unsigned char>(piksi_mode_t::spp));
    tf.piksi_microdelta_fp->set(0);
    
    // Initialize the estimate
    tf.piksi_time_fp->set(pan_epoch);
    tf.time_estimator.execute();

    TEST_ASSERT_TRUE(tf.time_valid_fp->get());
    TEST_ASSERT_FALSE(tf.time_reset_cmd_fp->get());

    // Ensure reset will be successfull even with valid data for a control cycle
    tf.time_reset_cmd_fp->set(true);
    tf.time_estimator.execute();

    TEST_ASSERT_FALSE(tf.time_valid_fp->get());
    TEST_ASSERT_FALSE(tf.time_reset_cmd_fp->get());

    // Ensure the estimate will initialize again
    tf.time_estimator.execute();

    TEST_ASSERT_TRUE(tf.time_valid_fp->get());
    TEST_ASSERT_FALSE(tf.time_reset_cmd_fp->get());
}

int test_control_task()
{
    UNITY_BEGIN();
    RUN_TEST(test_constructor);
    RUN_TEST(test_update);
    RUN_TEST(test_microdelta);
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
