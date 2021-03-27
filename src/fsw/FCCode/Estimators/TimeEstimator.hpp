#ifndef ESTIMATORS_TIME_ESTIMATOR_HPP_
#define ESTIMATORS_TIME_ESTIMATOR_HPP_

#include <common/GPSTime.hpp>

#include <fsw/FCCode/ControlTask.hpp>
#undef abs

/** @author Kyle Krol
 *
 *  @brief Provides an estimate of the current time given information from the
 *         Piksi.
 *
 *  The time estimate is considered invalid until the Piksi provides at least a
 *  single GPS time reading. From then on, the time estimate will always remain
 *  valid and update itself either from Piksi or based on the expected length of
 *  a control cycle.
 *
 *  Note that the time estimate isn't guranteed to be monotonically increasing
 *  and, as such, sequential time estimates shouldn't be used to calculate
 *  timestep sizes; use the control cycle length constants instead.
 */
class TimeEstimator : public ControlTask<void>
{
  public:
    TimeEstimator() = delete;
    TimeEstimator(TimeEstimator const &) = delete;
    TimeEstimator(TimeEstimator &&) = delete;
    TimeEstimator &operator=(TimeEstimator const &) = delete;
    TimeEstimator &operator=(TimeEstimator &&) = delete;

    ~TimeEstimator() = default;

    TimeEstimator(StateFieldRegistry &registry);

    void execute() override;

  protected:
    /* Take position, velocity, relative position, time, time delay, and current
     * static readings in from Piksi.
     */
    ReadableStateField<unsigned char> const *const piksi_state_fp;
    ReadableStateField<unsigned int> const *const piksi_microdelta_fp;
    ReadableStateField<gps_time_t> const *const piksi_time_fp;

    /* Outputs the current time estimate.
     */
    ReadableStateField<bool> time_valid_f;
    ReadableStateField<gps_time_t> time_gps_f;
    InternalStateField<unsigned long long> time_ns_f;
    InternalStateField<double> time_s_f;

    /* Outputs ephemeris information that's directly tied to the current time
     * estimate.
     */
    InternalStateField<lin::Vector3d> time_earth_w_f;
    InternalStateField<lin::Vector4d> time_earth_q_ecef_eci_f;
    InternalStateField<lin::Vector4d> time_earth_q_eci_ecef_f;

    /* Command to forcibly reset the time estimate.
     */
    WritableStateField<bool> time_reset_cmd_f;
};

#endif
