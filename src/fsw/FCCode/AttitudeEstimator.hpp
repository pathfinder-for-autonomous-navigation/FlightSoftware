#ifndef ATTITUDE_ESTIMATOR_HPP_
#define ATTITUDE_ESTIMATOR_HPP_

#include "TimedControlTask.hpp"

#include <gnc/attitude_estimator.hpp>
#include <lin/core.hpp>

/** @brief Control task that estimates the spacecraft's attitude.
 *
 *  This control taks is essentially a wrapper around the attitude estimation
 *  algorithm provided by GNC (see gnc/attitude_estimator.hpp) that relies on
 *  inputs from the OrbitEstimator and ADCSBoxMonitor control tasks.
 *
 *  This control task itself can either be in one of two states: initialized or
 *  uninitialized.
 *
 *  In the initialized state, the filters state is "valid" and a filter update
 *  step cn succeed as long as time, position, magnetic field, and angular rate
 *  data is provided for a given control cycle (sun vector data is optional). If
 *  the data fails to be provided, the filter will fall back into an "invalid"
 *  state and must be reinitialized.
 *
 *  The filter is in an uninitialized state at startup. In this state, the control
 *  task will wait for time, position, magnetic field, and sun vector data to be
 *  provided. The filter will then attempt to initialize itself with that data. It
 *  is not guranteed to initialize (see the GNC documentation for more
 *  information).
 *
 *  The filter, when in the initialized state, will provide attitude, angular
 *  rate, and a state covariance criterion as outputs. If the filter is
 *  uninitialized, all estimation state fields will be set to NaN. */
class AttitudeEstimator : public TimedControlTask<void> {
   public:
    /**
     * @brief Construct a new Attitude Estimator.
     * 
     * @param registry     */
    AttitudeEstimator(StateFieldRegistry& registry);

    /** @brief Attempts to either initialize or update the attitude filter.
     *
     *  Pulls orbit estimates and sensor readings from the appropriate state
     *  fields to act as inputs to the filter.
     * 
     *  One thing to note here, is the mag_flag_f state field determines which
     *  magnetometer data is pulled from. When set to `false`, the first
     *  magnetometer is used. When set to `true`, the second magnetometer is used.
     *  If the chosen magnetometer fails to provide data, the other device is
     *  looked to.
     *
     *  See the class documentation for more information about this control task's
     *  behavior. */
    void execute() override;

   protected:
    /** @brief Time in seconds since the PAN epoch.
     *
     *  Input taken from the OrbitEstimator. */
    ReadableStateField<double> const *const time_fp;
    /** @brief Position of the satellite in the ECEF frame (meters).
     * 
     *  Input taken from the OrbitEstimator. */
    ReadableStateField<lin::Vector3d> const *const pos_fp;

    /** @brief Magnetic field readings in the body frame (Tesla).
     * 
     *  Input taken from the ADCSBoxMonitor.
     *  @{ */
    ReadableStateField<lin::Vector3f> const *const mag1_vec_fp;
    ReadableStateField<lin::Vector3f> const *const mag2_vec_fp;
    /** @} */
    /** @brief Sun vector reading in the body frame (unit vector).
     *
     *  Input taken from the ADCSBoxMonitor. */
    ReadableStateField<lin::Vector3f> const *const ssa_vec_fp;
    /** @brief Angular rate reading in the body frame (randians per second).
     *  
     *  Input taken rom the ADCSBoxMonitor. */
    ReadableStateField<lin::Vector3f> const *const gyr_vec_fp;

    /** @brief Selects which magnetometer to, by default, read from.
     *
     *  If the selected magnetometer doesn't present a reading on a given control
     *  cycle, the other magnetometer will be polled.
     * 
     *  The default values is `false`. */
    WritableStateField<bool> mag_flag_f;

    /**
     * @brief Whichever selected magnetometer value will be written to this field.
     * 
     * Internal because the information needed will 
     * already be downlinked from the BoxMonitor fields.
     */
    InternalStateField<lin::Vector3f> b_body_f;

    /** @brief Estimated attitude quaternion.
     *
     *  Transforms from ECI to the body frame. If no attitude estimate exists, the
     *  field will be set to NaN. */
    ReadableStateField<lin::Vector4f> q_body_eci_est_f;
    /** @brief Estimated angular rate of the spacecraft in the body frame
     *         (radians per second).
     *
     *  If no angular rate estimate exists, the field will be set to NaN. */
    ReadableStateField<lin::Vector3f> w_body_est_f;
    /** @brief Frobenius norm of the estimated state covariance.
     *
     *  If no state covariance estimate exists, the field will be set to NaN. */
    ReadableStateField<float> fro_P_est_f;

    /** @internal Attitude estimator interface adaptars and calculation buffer.
     *  @{ */
    gnc::AttitudeEstimatorData data;
    gnc::AttitudeEstimatorState state;
    gnc::AttitudeEstimate estimate;
    /** @} */

    /** @internal Internal helper function(s).
     *  @{ */
    void _nan_estimate();
    /** @} */
};

#endif
