#ifndef ATTITUDE_ESTIMATOR_HPP_
#define ATTITUDE_ESTIMATOR_HPP_

#include <ControlTask.hpp>

/**
 * @brief Using raw sensor inputs, determine the attitude and angular state
 * of the spacecraft.
 */
class AttitudeEstimator : public ControlTask<void> {
   public:
    /**
     * @brief Construct a new Attitude Estimator.
     * 
     * @param registry 
     */
    AttitudeEstimator(StateFieldRegistry& registry);

    /**
     * @brief Using raw sensor inputs, determine the attitude and angular state
     * of the spacecraft.
     */
    void execute() override;

   protected:
    /**
     * @brief Inputs collected from ADCSBoxMonitor.
     */
    //! Sun vector in the body frame.
    std::shared_ptr<ReadableStateField<f_vector_t>> sun_vec_body_rd_fp;
    //! Validity of the sun vector that was computed.
    std::shared_ptr<ReadableStateField<bool>> sun_vec_body_rd_valid_fp;
    //! Magnetic field vector in the body frame.
    std::shared_ptr<ReadableStateField<f_vector_t>> mag_vec_body_rd_fp;
    //! Gyroscope vector in the body frame.
    std::shared_ptr<ReadableStateField<f_vector_t>> gyr_vec_body_rd_fp;
    //! Reaction wheel speeds.
    std::shared_ptr<ReadableStateField<f_vector_t>> rwa_rd_fp;
    //! Position of current satellite, in the ECI frame.
    std::shared_ptr<ReadableStateField<f_vector_t>> r1_vec_eci_fp;
    //! Position of other satellite, in the ECI frame.
    std::shared_ptr<ReadableStateField<f_vector_t>> r2_vec_eci_fp;
    
    /**
     * @brief Constant parameters for calculation.
     */
    static const f_vector_t gyro_bias;   /**< Gyroscope bias **/
    static const f_vector_t mag_bias;    /**< Magnetometer bias **/
    static const std::array<float, 6> I; /**< Moment of inertia **/

    /**
     * @brief Estimation outputs.
     */
    //! True if satellite is in eclipse.
    ReadableStateField<bool> in_eclipse_f;
    //! Spacecraft angular momentum vector, in the body frame.
    ReadableStateField<f_vector_t> h_vec_body_f;
    //! Quaternion representing the rotation from the body frame to the ECI frame.
    ReadableStateField<f_quat_t> body_to_eci_f;
};

#endif
