#ifndef ESTIMATORS_ATTITUDE_ESTIMATOR_HPP_
#define ESTIMATORS_ATTITUDE_ESTIMATOR_HPP_

#include <fsw/FCCode/ControlTask.hpp>
#undef abs

#include <gnc/attitude_estimator.hpp>

#include <lin/core.hpp>

/** @author Kyle Krol
 *
 *  @brief Control task that estimates the spacecraft's attitude.
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
 *  uninitialized, all estimation state fields will be set to NaN.
 */
class AttitudeEstimator : public ControlTask<void>
{
  public:
    AttitudeEstimator() = delete;
    AttitudeEstimator(AttitudeEstimator const &) = delete;
    AttitudeEstimator(AttitudeEstimator &&) = delete;
    AttitudeEstimator &operator=(AttitudeEstimator const &) = delete;
    AttitudeEstimator &operator=(AttitudeEstimator &&) = delete;

    ~AttitudeEstimator() = default;

    AttitudeEstimator(StateFieldRegistry &registry);

    /**
     * @brief Collect ADCS commander state field.
     */
    void init();

    void execute() override;

  protected:
    void _execute();

    /*
     */
    ReadableStateField<bool> const *const time_valid_fp;
    InternalStateField<double> const *const time_s_fp;

    /*
     */
    ReadableStateField<bool> const *const orbit_valid_fp;
    ReadableStateField<lin::Vector3d> const *const orbit_pos_fp;

    /*
     */
    ReadableStateField<bool> const *const adcs_gyr_functional_fp;
    ReadableStateField<lin::Vector3f> const *const adcs_gyr_fp;
    ReadableStateField<bool> const *const adcs_mag1_functional_fp;
    ReadableStateField<lin::Vector3f> const *const adcs_mag1_fp;
    ReadableStateField<bool> const *const adcs_mag2_functional_fp;
    ReadableStateField<lin::Vector3f> const *const adcs_mag2_fp;
    ReadableStateField<unsigned char> const *const adcs_ssa_mode_fp;
    ReadableStateField<lin::Vector3f> const *const adcs_ssa_fp;

    /*
     */
    InternalStateField<bool> attitude_estimator_b_valid_f;
    InternalStateField<lin::Vector3f> attitude_estimator_b_body_f;

    /*
     */
    ReadableStateField<bool> attitude_estimator_valid_f;
    ReadableStateField<lin::Vector4f> attitude_estimator_q_body_eci_f;
    ReadableStateField<lin::Vector3f> attitude_estimator_p_body_eci_sigma_f;
    InternalStateField<lin::Vector3f> attitude_estimator_w_body_f;
    ReadableStateField<lin::Vector3f> attitude_estimator_w_bias_body_f;
    ReadableStateField<lin::Vector3f> attitude_estimator_w_bias_body_sigma_f;
    ReadableStateField<lin::Vector3f> attitude_estimator_L_body_f;

    /*
     */
    WritableStateField<bool> attitude_estimator_reset_cmd_f;
    WritableStateField<bool> attitude_estimator_mag_flag_f;

    ReadableStateField<lin::Vector3f> const *adcs_cmd_mtr_cmd;

    /*
     */
    Fault attitude_estimator_fault;

  private:
    gnc::AttitudeEstimatorData _data;
    gnc::AttitudeEstimatorState _state;
    gnc::AttitudeEstimate _estimate;

    lin::Vector3f _cycle_slip_mtr_cmd;
};

#endif
