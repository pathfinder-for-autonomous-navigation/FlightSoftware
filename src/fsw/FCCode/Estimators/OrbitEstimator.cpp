#include "OrbitEstimator.hpp"

#include <fsw/FCCode/constants.hpp>
#include <fsw/FCCode/piksi_mode_t.enum>

#include <gnc/config.hpp>
#include <gnc/constants.hpp>

#include <lin/core.hpp>
#include <lin/generators.hpp>
#include <lin/references.hpp>

// Estimator process noise
TRACKED_CONSTANT_SC(double, Orbit_sqrtQ_r, 0.1);
TRACKED_CONSTANT_SC(double, Orbit_sqrtQ_v, 0.1);
static constexpr lin::Vectord<6> sqrtQ_diag {Orbit_sqrtQ_r, Orbit_sqrtQ_r,
        Orbit_sqrtQ_r, Orbit_sqrtQ_v, Orbit_sqrtQ_v, Orbit_sqrtQ_v};
static constexpr lin::Matrixd<6, 6> sqrtQ = lin::diag(sqrtQ_diag);

// Estimator sensor noise
TRACKED_CONSTANT_SC(double, Orbit_sqrtR_r, 5.0);
TRACKED_CONSTANT_SC(double, Orbit_sqrtR_v, 5.0);
static constexpr lin::Vectord<6> sqrtR_diag {Orbit_sqrtR_r, Orbit_sqrtR_r,
        Orbit_sqrtR_r, Orbit_sqrtR_v, Orbit_sqrtR_v, Orbit_sqrtR_v};
static constexpr lin::Matrixd<6, 6> sqrtR = lin::diag(sqrtR_diag);

OrbitEstimator::OrbitEstimator(StateFieldRegistry &registry)
    : ControlTask<void>(registry),
      piksi_state_fp(FIND_READABLE_FIELD(unsigned char, piksi.state)),
      piksi_microdelta_fp(FIND_READABLE_FIELD(unsigned int, piksi.microdelta)),
      piksi_pos_fp(FIND_READABLE_FIELD(lin::Vector3d, piksi.pos)),
      piksi_vel_fp(FIND_READABLE_FIELD(lin::Vector3d, piksi.vel)),
      time_valid_fp(FIND_READABLE_FIELD(bool, time.valid)),
      time_ns_fp(FIND_INTERNAL_FIELD(unsigned long long, time.ns)),
      time_earth_w_fp(FIND_INTERNAL_FIELD(lin::Vector3d, time.earth.w)),
      orbit_valid_f("orbit.valid", Serializer<bool>()),
      orbit_pos_f("orbit.pos", Serializer<lin::Vector3d>(6771000, 6921000, 28)),
      orbit_pos_sigma_f("orbit.pos_sigma", Serializer<lin::Vector3d>(0, 100, 12)),
      orbit_vel_f("orbit.vel", Serializer<lin::Vector3d>(7570, 7685, 19)),
      orbit_vel_sigma_f("orbit.vel_sigma", Serializer<lin::Vector3d>(0, 50, 12)),
      orbit_reset_cmd_f("orbit.reset_cmd", Serializer<bool>())
{
    add_readable_field(orbit_valid_f);
    add_readable_field(orbit_pos_f);
    add_readable_field(orbit_pos_sigma_f);
    add_readable_field(orbit_vel_f);
    add_readable_field(orbit_vel_sigma_f);
    add_writable_field(orbit_reset_cmd_f);

    orbit_valid_f.set(false);
    orbit_pos_f.set(lin::zeros<lin::Vector3d>());
    orbit_pos_sigma_f.set(lin::zeros<lin::Vector3d>());
    orbit_vel_f.set(lin::zeros<lin::Vector3d>());
    orbit_vel_sigma_f.set(lin::zeros<lin::Vector3d>());
    orbit_reset_cmd_f.set(false);
}

void OrbitEstimator::execute()
{
    auto const should_reset = !time_valid_fp->get() || orbit_reset_cmd_f.get();

    if (should_reset)
    {
        orbit_valid_f.set(false);
        orbit_reset_cmd_f.set(false);

        _estimate = orb::OrbitEstimate();

        return;
    }

    auto const piksi_mode = static_cast<piksi_mode_t>(piksi_state_fp->get());
    auto const piksi_dns = static_cast<unsigned long long>(1000 * piksi_microdelta_fp->get());
    auto const w_earth_ecef = time_earth_w_fp->get();

    double _;
    switch (piksi_mode)
    {
        // Received a valid position and velocity reading from Piksi
        case piksi_mode_t::spp:
        case piksi_mode_t::float_rtk:
        case piksi_mode_t::fixed_rtk:
        {
            // Pass MINGPSTIME_NS until the timestamp is removed from the class
            orb::OrbitEstimate measurement(orb::MINGPSTIME_NS, piksi_pos_fp->get(),
                    piksi_vel_fp->get(), sqrtR);

            if (piksi_dns)
            {
                measurement.shortupdate(piksi_dns, w_earth_ecef, sqrtQ, _);
            }

            if (_estimate.valid())
            {
                _estimate.shortupdate(PAN::control_cycle_time_ns, w_earth_ecef,
                        measurement.recef(), measurement.vecef(), sqrtQ, measurement.S(), _);
            }
            else
            {
                _estimate = measurement;
            }
            break;
        }

        // No valid reading from Piksi
        default:
            if (_estimate.valid())
            {
                _estimate.shortupdate(PAN::control_cycle_time_ns, w_earth_ecef, sqrtQ, _);
            }
            break;
    }

    orbit_valid_f.set(_estimate.valid());
    if (_estimate.valid())
    {
        auto const S = _estimate.S();

        orbit_pos_f.set(_estimate.recef());
        orbit_pos_sigma_f.set(lin::ref<lin::Vector3d>(lin::diag(S), 0, 0));
        orbit_vel_f.set(_estimate.vecef());
        orbit_vel_sigma_f.set(lin::ref<lin::Vector3d>(lin::diag(S), 3, 0));
    }
}
