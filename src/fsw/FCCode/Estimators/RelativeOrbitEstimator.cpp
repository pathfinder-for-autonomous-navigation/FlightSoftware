#include "RelativeOrbitEstimator.hpp"

#include <fsw/FCCode/constants.hpp>
#include <fsw/FCCode/Estimators/rel_orbit_state_t.enum>
#include <fsw/FCCode/piksi_mode_t.enum>

#include <gnc/config.hpp>
#include <gnc/environment.hpp>

#include <lin/core.hpp>
#include <lin/generators.hpp>
#include <lin/math.hpp>
#include <lin/references.hpp>

#if (defined(UNIT_TEST) && defined(DESKTOP))
#include <cstdio>
#define DD(fstring, ...)                                       \
   std::printf("[RelativeOrbitEstimator.cpp:%d]: ", __LINE__); \
   std::printf(fstring, ##__VA_ARGS__);                        \
   std::printf("\n")
#else
#define DD(fstring, ...) static_assert(true, "")
#endif

// Estimator process noise
TRACKED_CONSTANT_SC(double, RelOrbit_sqrtQ_r, 1.0e-8);
TRACKED_CONSTANT_SC(double, RelOrbit_sqrtQ_v, 1.0e-2);
static constexpr lin::Vectord<6> sqrtQ_diag{RelOrbit_sqrtQ_r, RelOrbit_sqrtQ_r,
                                            RelOrbit_sqrtQ_r, RelOrbit_sqrtQ_v, RelOrbit_sqrtQ_v, RelOrbit_sqrtQ_v};
static constexpr lin::Matrixd<6, 6> sqrtQ = lin::diag(sqrtQ_diag);

// Estimator sensor noise
TRACKED_CONSTANT_SC(double, RelOrbit_sqrtR_r, 1.0e-1);
static constexpr lin::Vector3d sqrtR_diag =
    RelOrbit_sqrtR_r * lin::ones<lin::Vector3d>();
static constexpr lin::Matrix3x3d sqrtR = lin::diag(sqrtR_diag);

static inline auto get_gps_zero()
{
   return gps_time_t(0, 0, 0);
}

static inline constexpr auto cast_state(rel_orbit_state_t state)
{
   return static_cast<unsigned char>(state);
}

RelativeOrbitEstimator::RelativeOrbitEstimator(StateFieldRegistry &registry)
    : ControlTask<void>(registry),
      piksi_state_fp(FIND_READABLE_FIELD(unsigned char, piksi.state)),
      piksi_microdelta_fp(FIND_READABLE_FIELD(unsigned int, piksi.microdelta)),
      piksi_pos_fp(FIND_READABLE_FIELD(lin::Vector3d, piksi.pos)),
      piksi_vel_fp(FIND_READABLE_FIELD(lin::Vector3d, piksi.vel)),
      piksi_baseline_pos_fp(FIND_READABLE_FIELD(lin::Vector3d, piksi.baseline_pos)),
      time_valid_fp(FIND_READABLE_FIELD(bool, time.valid)),
      time_gps_fp(FIND_READABLE_FIELD(gps_time_t, time.gps)),
      time_earth_w_fp(FIND_INTERNAL_FIELD(lin::Vector3d, time.earth.w)),
      orbit_valid_fp(FIND_READABLE_FIELD(bool, orbit.valid)),
      orbit_pos_fp(FIND_READABLE_FIELD(lin::Vector3d, orbit.pos)),
      orbit_vel_fp(FIND_READABLE_FIELD(lin::Vector3d, orbit.vel)),
      rel_orbit_state_f("rel_orbit.state", Serializer<unsigned char>(2)),
      rel_orbit_pos_f("rel_orbit.pos", Serializer<lin::Vector3d>(6771000, 6921000, 28)),
      rel_orbit_vel_f("rel_orbit.vel", Serializer<lin::Vector3d>(7570, 7685, 19)),
      rel_orbit_rel_pos_f("rel_orbit.rel_pos", Serializer<lin::Vector3d>(0, 2000, 22)),
      rel_orbit_rel_pos_sigma_f("rel_orbit.rel_pos_sigma", Serializer<lin::Vector3d>(0, 10, 12)),
      rel_orbit_rel_vel_f("rel_orbit.rel_vel", Serializer<lin::Vector3d>(0, 11, 14)),
      rel_orbit_rel_vel_sigma_f("rel_orbit.rel_vel_sigma", Serializer<lin::Vector3d>(0, 1, 12)),
      rel_orbit_reset_cmd_f("rel_orbit.reset_cmd", Serializer<bool>()),
      rel_orbit_uplink_time_f("rel_orbit.uplink.time", Serializer<gps_time_t>()),
      rel_orbit_uplink_pos_f("rel_orbit.uplink.pos", Serializer<lin::Vector3d>(6771000, 6921000, 28)),
      rel_orbit_uplink_vel_f("rel_orbit.uplink.vel", Serializer<lin::Vector3d>(7570, 7685, 19)),
      _uplink_t(get_gps_zero()),
      _have_previous_baseline(false),
      _cycles_without_rtk(0)
{
   add_readable_field(rel_orbit_state_f);
   add_readable_field(rel_orbit_pos_f);
   add_readable_field(rel_orbit_vel_f);
   add_readable_field(rel_orbit_rel_pos_f);
   add_readable_field(rel_orbit_rel_pos_sigma_f);
   add_readable_field(rel_orbit_rel_vel_f);
   add_readable_field(rel_orbit_rel_vel_sigma_f);
   add_writable_field(rel_orbit_reset_cmd_f);
   add_writable_field(rel_orbit_uplink_time_f);
   add_writable_field(rel_orbit_uplink_pos_f);
   add_writable_field(rel_orbit_uplink_vel_f);

   rel_orbit_state_f.set(cast_state(rel_orbit_state_t::invalid));
   rel_orbit_pos_f.set(lin::zeros<lin::Vector3d>());
   rel_orbit_vel_f.set(lin::zeros<lin::Vector3d>());
   rel_orbit_rel_pos_f.set(lin::zeros<lin::Vector3d>());
   rel_orbit_rel_pos_sigma_f.set(lin::zeros<lin::Vector3d>());
   rel_orbit_rel_vel_f.set(lin::zeros<lin::Vector3d>());
   rel_orbit_rel_vel_sigma_f.set(lin::zeros<lin::Vector3d>());
   rel_orbit_reset_cmd_f.set(false);
   rel_orbit_uplink_time_f.set(get_gps_zero());
   rel_orbit_uplink_pos_f.set(lin::zeros<lin::Vector3d>());
   rel_orbit_uplink_vel_f.set(lin::zeros<lin::Vector3d>());
}

void RelativeOrbitEstimator::execute()
{
   /* Reset the relative orbit estimator if commanded to, there isn't a valid
     * time estimate, or there isn't a valid orbit estimate.
     */
   {
      auto const should_reset = !time_valid_fp->get() ||
                                !orbit_valid_fp->get() || rel_orbit_reset_cmd_f.get();

      if (should_reset)
      {
         DD("Resetting the relative orbit estimator:");
         DD("\ttime.valid          = %d", time_valid_fp->get());
         DD("\torbit.valid         = %d", orbit_valid_fp->get());
         DD("\trel_orbit.reset_cmd = %d", rel_orbit_reset_cmd_f.get());

         rel_orbit_state_f.set(cast_state(rel_orbit_state_t::invalid));
         rel_orbit_reset_cmd_f.set(false);

         _uplink_t = get_gps_zero();
         _relative_orbit = gnc::RelativeOrbitEstimate();
         _orbit = orb::Orbit();

         return;
      }
   }

   auto const piksi_mode = static_cast<piksi_mode_t>(piksi_state_fp->get());
   auto const piksi_dns = static_cast<unsigned long long>(1000 * piksi_microdelta_fp->get());
   auto const time_gps = time_gps_fp->get();
   auto const time_earth_w = time_earth_w_fp->get();

   /* Handle potential uplinks.
     *
     * First, we need to populate the absolute orbital state of the other
     * spacecraft if there is a currently queued update that's at most two
     * control cycles past the current time estimate.
     *
     * Note these two subtle yet important implementation details:
     *
     *  1. A precondition that makes the rest of the control easier to write is
     *     that the absolute orbit estimate of the other spacecraft is either
     *     valid and correlating with a time a control cycle ago or invalid. The
     *     uplinks section is implemented with this in mind hence the potential
     *     propagation backward in time.
     *  2. While the uplinks may set the absolute orbital state of the other
     *     spacecraft, this will be overwritten if the relative estimator is up
     *     and running alongside a valid orbit estimate. This essentially means
     *     we're wasting uplinked information; however, that doesn't really
     *     matter as a direct relative estimate should be more meaningful.
     */
   {
      auto const have_queued_uplink = _uplink_t > get_gps_zero();
      auto const its_stale = time_gps > _uplink_t;

      if (have_queued_uplink && its_stale)
      {
         auto const ns = static_cast<unsigned long long>(time_gps - _uplink_t);

         DD("Queued uplink stale by %lld ns.", ns);

         if (ns < 2u * PAN::control_cycle_time_ns)
         {
            auto const signed_ns = static_cast<signed int>(ns) -
                                   static_cast<signed int>(PAN::control_cycle_time_ns);

            double _;
            _orbit = orb::Orbit(orb::MINGPSTIME_NS, _uplink_r, _uplink_v);
            _orbit.shortupdate(signed_ns, time_earth_w, _);

            DD("Initializing absolute orbital state with queued uplink:");
            DD("\tr = %f,%f,%f", _uplink_r(0), _uplink_r(1), _uplink_r(2));
            DD("\tv = %f,%f,%f", _uplink_v(0), _uplink_v(1), _uplink_v(2));
         }

         _uplink_t = get_gps_zero();
      }
   }
   /* Second, check if there is an uplink from the ground that can be moved in
     * to the queue.
     */
   {
      auto const uplink_time = rel_orbit_uplink_time_f.get();
      auto const have_new_uplink = uplink_time > get_gps_zero();

      if (have_new_uplink)
      {
         auto const have_uplink_queued = _uplink_t > get_gps_zero();
         auto const new_uplink_is_ahead = uplink_time > time_gps;
         auto const new_uplink_is_closer = uplink_time < _uplink_t;

         DD("New uplink received:");
         DD("\thave_uplink_queued   = %i", have_uplink_queued);
         DD("\tnew_uplink_is_ahead  = %i", new_uplink_is_ahead);
         DD("\tnew_uplink_is_closer = %i", new_uplink_is_closer);

         if (new_uplink_is_ahead && (!have_uplink_queued || new_uplink_is_closer))
         {
            _uplink_t = rel_orbit_uplink_time_f.get();
            _uplink_r = rel_orbit_uplink_pos_f.get();
            _uplink_v = rel_orbit_uplink_vel_f.get();

            DD("Moving new uplink into the queue:");
            DD("\tr = %f,%f,%f", _uplink_r(0), _uplink_r(1), _uplink_r(2));
            DD("\tv = %f,%f,%f", _uplink_v(0), _uplink_v(1), _uplink_v(2));
         }
      }

      rel_orbit_uplink_time_f.set(get_gps_zero());
   }

   auto const orbit_pos = orbit_pos_fp->get();
   auto const orbit_vel = orbit_vel_fp->get();

   /* Handle the relative orbit estimate.
     *
     * Essentially here we will try and update the relative orbit estimate.
     */
   switch (piksi_mode)
   {
   case piksi_mode_t::fixed_rtk:
   {
      auto const piksi_baseline_pos = piksi_baseline_pos_fp->get();

      // If the estimate is already valid, then we run an update step.
      if (_relative_orbit.valid())
      {
         _cycles_without_rtk = 0;

         _relative_orbit.update(PAN::control_cycle_time_ns - piksi_dns, time_earth_w,
                                orbit_pos, orbit_vel, -piksi_baseline_pos, sqrtQ, sqrtR);
         _relative_orbit.update(piksi_dns, time_earth_w, orbit_pos, orbit_vel, sqrtQ);
      }
      // Otherwise, attempt to initialize the estimate.
      else
      {
         if (_have_previous_baseline)
         {
            _previous_baseline_ns += PAN::control_cycle_time_ns;

            if (_previous_baseline_ns <= 10 * PAN::control_cycle_time_ns)
            {
               lin::Matrixd<6, 6> S;
               lin::ref<lin::Matrix3x3d>(S, 0, 0) = sqrtR;
               lin::ref<lin::Matrix3x3d>(S, 3, 3) =
                   lin::sqrt(2.0e9 / double(_previous_baseline_ns)) * sqrtR;

               lin::Vector3d baseline_dv =
                   1.0e9 * (piksi_baseline_pos - _previous_baseline_pos) / double(_previous_baseline_ns);

               DD("Initializing the relative orbit estimate with:");
               DD("\trel_pos = %f,%f,%f", -piksi_baseline_pos(0), -piksi_baseline_pos(1), -piksi_baseline_pos(2));
               DD("\trel_vel = %f,%f,%f", -baseline_dv(0), -baseline_dv(1), -baseline_dv(2));

               _relative_orbit = gnc::RelativeOrbitEstimate(
                   time_earth_w, orbit_pos, orbit_vel, -piksi_baseline_pos, -baseline_dv, S);

               _have_previous_baseline = false;
               break;
            }
         }
         _previous_baseline_pos = piksi_baseline_pos;
         _previous_baseline_ns = piksi_dns;
         _have_previous_baseline = true;
      }
      break;
   }

   default:
   {
      if (_relative_orbit.valid())
      {
         _cycles_without_rtk++;

         _relative_orbit.update(
             PAN::control_cycle_time_ns, time_earth_w, orbit_pos, orbit_vel, sqrtQ);
      }
   }
   }

   /* Mix the relative and absolute orbit measurements.
     *
     * At this point, we should have have a potentially up-to-date relative
     * orbit estimate and/or an up-to-date absolute orbital state for the other
     * spacecraft. We will favor the relative orbit estimate if present.
     */
   if (_relative_orbit.valid())
   {
      auto const rel_pos = _relative_orbit.dr_ecef();
      auto const rel_vel = _relative_orbit.dv_ecef();
      auto const S = _relative_orbit.S();
      auto const pos = (orbit_pos + rel_pos).eval();
      auto const vel = (orbit_vel + rel_vel).eval();

      // Overwrite any uplinked absolute orbit estimate
      _orbit = orb::Orbit(orb::MINGPSTIME_NS, pos, vel);

      rel_orbit_state_f.set(cast_state(rel_orbit_state_t::estimating));

      rel_orbit_pos_f.set(pos);
      rel_orbit_vel_f.set(vel);

      rel_orbit_rel_pos_f.set(rel_pos);
      rel_orbit_rel_pos_sigma_f.set(lin::ref<lin::Vector3d>(lin::diag(S), 0, 0));
      rel_orbit_rel_vel_f.set(rel_vel);
      rel_orbit_rel_vel_sigma_f.set(lin::ref<lin::Vector3d>(lin::diag(S), 3, 0));

      /* Limit the number of pure prediction steps we'll do before reverting
         * back to the full orbit propagator.
         */
      TRACKED_CONSTANT_SC(unsigned int, REL_ORBIT_CC_PREDICT_LIMIT, 5000);
      if (_cycles_without_rtk > REL_ORBIT_CC_PREDICT_LIMIT)
      {
         DD("Control cycle predict limit reached for relative orbit "
            "estimation; resetting the relative estimate:");
         DD("\t_orbit.valid() = %i", _orbit.valid());
         DD("\tpos     = %f,%f,%f", orbit_pos(0), orbit_pos(1), orbit_pos(2));
         DD("\tvel     = %f,%f,%f", orbit_vel(0), orbit_vel(1), orbit_vel(2));
         DD("\trel_pos = %f,%f,%f", rel_pos(0), rel_pos(1), rel_pos(2));
         DD("\trel_vel = %f,%f,%f", rel_vel(0), rel_vel(1), rel_vel(2));

         _cycles_without_rtk = 0;
         _relative_orbit = gnc::RelativeOrbitEstimate();

         // We also want to clear any potential uplink as the relative orbit
         // estimate should be more accurate.
         _uplink_t = get_gps_zero();
      }
   }
   /* Otherwise, with no relative orbit estimate all we can do is propegate the
     * absolute estimate if present
     */
   else if (_orbit.valid())
   {
      double _;
      _orbit.shortupdate(PAN::control_cycle_time_ns, time_earth_w, _);

      auto const pos = _orbit.recef();
      auto const vel = _orbit.vecef();

      rel_orbit_state_f.set(cast_state(rel_orbit_state_t::propagating));

      rel_orbit_pos_f.set(pos);
      rel_orbit_vel_f.set(vel);

      rel_orbit_rel_pos_f.set(pos - orbit_pos);
      rel_orbit_rel_vel_f.set(vel - orbit_vel);
   }
   /* Lastly, if nothing is valid we have an invalid relative orbit estimate.
     */
   else
   {
      rel_orbit_state_f.set(cast_state(rel_orbit_state_t::invalid));
   }
}