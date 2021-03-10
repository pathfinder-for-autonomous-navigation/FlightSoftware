#include "TimeEstimator.hpp"

#include <fsw/FCCode/constants.hpp>
#include <fsw/FCCode/piksi_mode_t.enum>

#include <gnc/config.hpp>
#include <gnc/constants.hpp>

TimeEstimator::TimeEstimator(StateFieldRegistry &registry)
    : ControlTask<void>(registry),
      piksi_state_fp(FIND_READABLE_FIELD(unsigned char, piksi.state)),
      piksi_microdelta_fp(FIND_READABLE_FIELD(unsigned int, piksi.microdelta)),
      piksi_time_fp(FIND_READABLE_FIELD(gps_time_t, piksi.time)),
      time_valid_f("time.valid", Serializer<bool>()),
      time_gps_f("time.gps", Serializer<gps_time_t>()),
      time_ns_f("time.ns"),
      time_s_f("time.s"),
      time_reset_cmd_f("time.reset_cmd", Serializer<bool>())
{
    add_readable_field(time_valid_f);
    add_internal_field(time_s_f);
    add_internal_field(time_ns_f);
    add_readable_field(time_gps_f);
    add_writable_field(time_reset_cmd_f);

    time_valid_f.set(false);
    time_reset_cmd_f.set(false);
}

void TimeEstimator::execute()
{
    if (time_reset_cmd_f.get())
    {
        time_valid_f.set(false);
        time_reset_cmd_f.set(false);
        return;
    }

    auto const piksi_mode = static_cast<piksi_mode_t>(piksi_state_fp->get());
    auto const piksi_dns = static_cast<unsigned long long>(1000 * piksi_microdelta_fp->get());
    auto const gps_epoch_ns = static_cast<unsigned long long>(gps_time_t(
            gnc::constant::init_gps_week_number, gnc::constant::init_gps_time_of_week,
            gnc::constant::init_gps_nanoseconds));

    switch (piksi_mode)
    {
        // Received a valid time reading from Piksi
        case piksi_mode_t::spp:
        case piksi_mode_t::float_rtk:
        case piksi_mode_t::fixed_rtk:
            time_gps_f.set(piksi_time_fp->get() + piksi_dns);
            time_valid_f.set(true);
            break;

        // No valid time reading from Piksi
        default:
            time_gps_f.set(time_gps_f.get() + PAN::control_cycle_time_ns);
            break;
    }

    // Update the other fields (means nothing it time is invalid anyway)
    time_ns_f.set(static_cast<unsigned long long>(time_gps_f.get()) - gps_epoch_ns);
    time_s_f.set(static_cast<double>(time_ns_f.get()) * 1.0e-9);
}
