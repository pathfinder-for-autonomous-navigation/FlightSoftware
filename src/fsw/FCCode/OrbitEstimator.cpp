#include "OrbitEstimator.hpp"

#include "constants.hpp"
#include "piksi_mode_t.enum"

#include <gnc/config.hpp>
#include <gnc/constants.hpp>
#include <gnc/environment.hpp>

#include <lin/core.hpp>
#include <lin/generators.hpp>
#include <lin/math.hpp>

// Estimator process noise
TRACKED_CONSTANT_SC(double, Orbit_sqrtQ_r, 0.1);
TRACKED_CONSTANT_SC(double, Orbit_sqrtQ_v, 0.1);
static constexpr lin::Vectord<6> sqrtQ_diag {Orbit_sqrtQ_r, Orbit_sqrtQ_r,
        Orbit_sqrtQ_r, Orbit_sqrtQ_v, Orbit_sqrtQ_v, Orbit_sqrtQ_v};
static constexpr lin::Matrixd<6, 6> sqrtQ {lin::diag(sqrtQ_diag)};

// Estimator sensor noise
TRACKED_CONSTANT_SC(double, Orbit_sqrtR_r, 5.0);
TRACKED_CONSTANT_SC(double, Orbit_sqrtR_v, 5.0);
static constexpr lin::Vectord<6> sqrtR_diag {Orbit_sqrtR_r, Orbit_sqrtR_r,
        Orbit_sqrtR_r, Orbit_sqrtR_v, Orbit_sqrtR_v, Orbit_sqrtR_v};
static constexpr lin::Matrixd<6, 6> sqrtR {lin::diag(sqrtR_diag)};

OrbitEstimator::OrbitEstimator(
        StateFieldRegistry &registry, unsigned int offset)
    : TimedControlTask<void>(registry, "orbit_estimator", offset),
      piksi_state_fp(FIND_READABLE_FIELD(unsigned char, piksi.state)),
      piksi_microdelta_fp(FIND_READABLE_FIELD(unsigned int, piksi.microdelta)),
      piksi_pos_fp(FIND_READABLE_FIELD(lin::Vector3d, piksi.pos)),
      piksi_vel_fp(FIND_READABLE_FIELD(lin::Vector3d, piksi.vel)),
      time_valid_fp(FIND_READABLE_FIELD(bool, time.valid)),
      time_s_fp(FIND_INTERNAL_FIELD(double, time.s)),
      time_ns_fp(FIND_INTERNAL_FIELD(unsigned long long, time.ns)),
      orbit_valid_f("orbit.valid", Serializer<bool>()),
      orbit_pos_f("orbit.pos", Serializer<lin::Vector3d>(6771000, 6921000, 28)),
      orbit_vel_f("orbit.vel", Serializer<lin::Vector3d>(7570, 7685, 19)),
      orbit_reset_f("orbit.reset", Serializer<bool>())
{
    add_readable_field(orbit_valid_f);
    add_readable_field(orbit_pos_f);
    add_readable_field(orbit_vel_f);
    add_writable_field(orbit_reset_f);

    orbit_valid_f.set(false);
    orbit_reset_f.set(false);
    orbit_pos_f.set(lin::zeros<lin::Vector3d>());
    orbit_vel_f.set(lin::zeros<lin::Vector3d>());
}

void OrbitEstimator::execute()
{
    if (!time_valid_fp->get() || orbit_reset_f.get())
    {
        orbit_valid_f.set(false);
        orbit_reset_f.set(false);
        return;
    }

    auto const piksi_mode = static_cast<piksi_mode_t>(piksi_state_fp->get());
    auto const piksi_dns = static_cast<unsigned long long>(1000 * piksi_microdelta_fp->get());
    auto const w_earth_ecef = [](double t) {
        lin::Vector3d w_earth_ecef;
        gnc::env::earth_angular_rate(t, w_earth_ecef);
        return w_earth_ecef;
    }(time_s_fp->get());

    double _;
    switch (piksi_mode)
    {
    // Received a valid position and velocity reading from Piksi
    case piksi_mode_t::spp:
    case piksi_mode_t::float_rtk:
    case piksi_mode_t::fixed_rtk:
        // Pass MINGPSTIME_NS until the timestampo is removed from the class
        orb::OrbitEstimate measurement(orb::MINGPSTIME_NS, piksi_pos_fp->get(),
                piksi_vel_fp->get(), sqrtR);
        measurement.shortupdate(piksi_dns, w_earth_ecef, sqrtQ, _);

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

    // No valid reading from Piksi
    default:
        if (_estimate.valid())
        {
            _estimate.shortupdate(PAN::control_cycle_time_ns, w_earth_ecef, sqrtQ, _);
        }
        break;
    }

    orbit_valid_f.set(_estimate.valid());
    if (orbit_valid_f.get())
    {
        orbit_pos_f.set(_estimate.recef());
        orbit_vel_f.set(_estimate.vecef());
    }
}
