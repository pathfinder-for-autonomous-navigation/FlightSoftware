#ifndef ESTIMATORS_RELATIVE_ORBIT_ESTIMATOR_HPP_
#define ESTIMATORS_RELATIVE_ORBIT_ESTIMATOR_HPP_

#include <common/GPSTime.hpp>

#include <fsw/FCCode/ControlTask.hpp>
#undef abs

#include <gnc/relative_orbit_estimate.hpp>

#include <lin/core.hpp>

#include <orb/Orbit.h>

/** @author Kyle Krol
 *
 *  @brief Provides an estimate of the other spacecraft's position and velocity
 *         relative to this one.
 */
class RelativeOrbitEstimator : public ControlTask<void>
{
  public:
    RelativeOrbitEstimator() = delete;
    RelativeOrbitEstimator(RelativeOrbitEstimator const &) = delete;
    RelativeOrbitEstimator(RelativeOrbitEstimator &&) = delete;
    RelativeOrbitEstimator &operator=(RelativeOrbitEstimator const &) = delete;
    RelativeOrbitEstimator &operator=(RelativeOrbitEstimator &&) = delete;

    ~RelativeOrbitEstimator() = default;

    RelativeOrbitEstimator(StateFieldRegistry &registry);

    void execute() override;

  protected:
    /* Take position, velocity, relative position, time, time delay, and current
     * static readings in from Piksi.
     */
    ReadableStateField<unsigned char> const *const piksi_state_fp;
    ReadableStateField<unsigned int> const *const piksi_microdelta_fp;
    ReadableStateField<lin::Vector3d> const *const piksi_pos_fp;
    ReadableStateField<lin::Vector3d> const *const piksi_vel_fp;
    ReadableStateField<lin::Vector3d> const *const piksi_baseline_pos_fp;

    /* Take the current time estimate from the time estimator.
     */
    ReadableStateField<bool> const *const time_valid_fp;
    ReadableStateField<gps_time_t> const *const time_gps_fp;
    InternalStateField<lin::Vector3d> const *const time_earth_w_fp;

    /* Take the current position and velocity estimates in from the orbit
     * estimator.
     */
    ReadableStateField<bool> const *const orbit_valid_fp;
    ReadableStateField<lin::Vector3d> const *const orbit_pos_fp;
    ReadableStateField<lin::Vector3d> const *const orbit_vel_fp;

    /* Outputs for relative position and velocity of the other spacecraft in
     * ECEF.
     * 
     * The estimate's sigma values are valid only if the relative orbit state is
     * set to estimating.
     */
    ReadableStateField<unsigned char> rel_orbit_state_f;
    ReadableStateField<lin::Vector3d> rel_orbit_pos_f;
    ReadableStateField<lin::Vector3d> rel_orbit_vel_f;
    ReadableStateField<lin::Vector3d> rel_orbit_rel_pos_f;
    ReadableStateField<lin::Vector3d> rel_orbit_rel_pos_sigma_f;
    ReadableStateField<lin::Vector3d> rel_orbit_rel_vel_f;
    ReadableStateField<lin::Vector3d> rel_orbit_rel_vel_sigma_f;

    /* Command to forcibly reset the relative orbit estimate.
     */
    WritableStateField<bool> rel_orbit_reset_cmd_f;

    /* Uplinks fields for the timestamped position and velocity of the other
     * spacecraft.
     */
    WritableStateField<gps_time_t> rel_orbit_uplink_time_f;
    WritableStateField<lin::Vector3d> rel_orbit_uplink_pos_f;
    WritableStateField<lin::Vector3d> rel_orbit_uplink_vel_f;

  private:
    gnc::RelativeOrbitEstimate _relative_orbit;
    orb::Orbit _orbit;

    gps_time_t _uplink_t;
    lin::Vector3d _uplink_r, _uplink_v;

    bool _have_previous_baseline;
    lin::Vector3d _previous_baseline_pos;
    unsigned int _previous_baseline_ns;

    unsigned int _cycles_without_rtk;
};

#endif
