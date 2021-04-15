#ifndef ESTIMATORS_ORBIT_ESTIMATOR_HPP_
#define ESTIMATORS_ORBIT_ESTIMATOR_HPP_

#include <fsw/FCCode/ControlTask.hpp>
#undef abs

#include <lin/core.hpp>

#include <orb/OrbitEstimate.hpp>

/** @author Kyle Krol
 *
 *  @brief Provides an estimate of this spacecraft's position and velocity given
 *         a time estimate and information from the Piksi.
 *
 *  The time estimate must be valid for the orbit estimate to be considered
 *  valid. Resetting the time estimate will cause the orbit estimate to be reset
 *  as well.
 *
 *  The position and velocity of the spacecraft are provided in ECEF.
 */
class OrbitEstimator : public ControlTask<void>
{
  public:
    OrbitEstimator() = delete;
    OrbitEstimator(OrbitEstimator const &) = delete;
    OrbitEstimator(OrbitEstimator &&) = delete;
    OrbitEstimator &operator=(OrbitEstimator const &) = delete;
    OrbitEstimator &operator=(OrbitEstimator &&) = delete;

    ~OrbitEstimator() = default;

    OrbitEstimator(StateFieldRegistry &registry);

    void execute() override;

  protected:
    /* Take position, velocity, time, time delay, and current state readings in
     * from Piksi.
     */
    ReadableStateField<unsigned char> const *const piksi_state_fp;
    ReadableStateField<unsigned int> const *const piksi_microdelta_fp;
    ReadableStateField<lin::Vector3d> const *const piksi_pos_fp;
    ReadableStateField<lin::Vector3d> const *const piksi_vel_fp;

    /* Take the current time estimate from the time estimator.
     */
    ReadableStateField<bool> const *const time_valid_fp;
    InternalStateField<unsigned long long> const *const time_ns_fp;
    InternalStateField<lin::Vector3d> const *const time_earth_w_fp;

    /* Outputs for current position and velocity estimates of this satellite.
     */
    ReadableStateField<bool> orbit_valid_f;
    ReadableStateField<lin::Vector3d> orbit_pos_f;
    ReadableStateField<lin::Vector3d> orbit_pos_sigma_f;
    ReadableStateField<lin::Vector3d> orbit_vel_f;
    ReadableStateField<lin::Vector3d> orbit_vel_sigma_f;

    /* Command to forcibly reset the orbit estimate.
     */
    WritableStateField<bool> orbit_reset_cmd_f;

  private:
    orb::OrbitEstimate _estimate;
};

#endif
