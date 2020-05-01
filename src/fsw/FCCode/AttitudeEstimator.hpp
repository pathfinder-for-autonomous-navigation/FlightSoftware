#ifndef ATTITUDE_ESTIMATOR_HPP_
#define ATTITUDE_ESTIMATOR_HPP_

#include "TimedControlTask.hpp"

#include <gnc/attitude_estimator.hpp>
#include <lin/core.hpp>

/**
 * @brief Using raw sensor inputs, determine the attitude and angular state
 * of the spacecraft.
 */
class AttitudeEstimator : public TimedControlTask<void> {
   public:
    /**
     * @brief Construct a new Attitude Estimator.
     * 
     * @param registry
     * @param offset
     */
    AttitudeEstimator(StateFieldRegistry& registry, unsigned int offset);

    /**
     * @brief Using raw sensor inputs, determine the attitude and angular state
     * of the spacecraft.
     */
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
     *  cycle, the other magnetometer will be polled. */
    WritableStateField<bool> mag_flag_f;

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

    /** @internal Internal helper functions.
     *  @{ */
    void _nan_estimate();
    /** @} */
};

#endif
