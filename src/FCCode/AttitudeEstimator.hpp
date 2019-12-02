#ifndef ATTITUDE_ESTIMATOR_HPP_
#define ATTITUDE_ESTIMATOR_HPP_

#include "../../.pio/libdeps/teensy35_hootl/psim/src/gnc_attitude_estimation.cpp"
#include <TimedControlTask.hpp>

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
     */
    AttitudeEstimator(StateFieldRegistry& registry, unsigned int offset);

    /**
     * @brief Using raw sensor inputs, determine the attitude and angular state
     * of the spacecraft.
     */
    void execute() override;

    void set_data();

    void set_estimate();

   protected:
    /**
     * @brief Inputs collected from Piksi and ADCSBoxMonitor.
     */

    ReadableStateField<gps_time_t>* pan_epoch_fp;

    //! Time from Piksi
    ReadableStateField<gps_time_t>* piksi_time_fp;
    //! Position of this satellite, Vector of doubles
    ReadableStateField<d_vector_t>* pos_vec_ecef_fp;
    //! Sun vector of this satellite, in the body frame.
    ReadableStateField<f_vector_t>* ssa_vec_rd_fp;
    //! Magnetic field vector of this satellite in the body frame.
    ReadableStateField<f_vector_t>* mag_vec_fp;
    

    // /**
    //  * @brief Constant parameters for calculation.
    //  */
    // WritableStateField<f_vector_t> gyro_bias_fp;   /**< Gyroscope bias **/
    // WritableStateField<f_vector_t> mag_bias_fp;    /**< Magnetometer bias **/

    /**
     * @brief Estimation outputs.
     */
    // //! True if satellite is in eclipse.
    // ReadableStateField<bool> in_eclipse_f;
    // //! Spacecraft angular momentum vector, in the body frame.
    // ReadableStateField<f_vector_t> h_vec_body_f;
    // //! Quaternion representing the rotation from the body frame to the ECI frame.
    // ReadableStateField<f_quat_t> body_to_eci_f;

    //kyle's structs
    gnc::AttitudeEstimatorData data;
    gnc::AttitudeEstimatorState state;
    gnc::AttitudeEstimate estimate;

    //AttitudeEstimate
    Serializer<f_quat_t> q_body_eci_sr;
    ReadableStateField<f_quat_t> q_body_eci_f;
    Serializer<f_vector_t> w_body_sr;
    ReadableStateField<f_vector_t> w_body_f;

};

#endif
