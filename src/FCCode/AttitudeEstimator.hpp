#ifndef ATTITUDE_ESTIMATOR_HPP_
#define ATTITUDE_ESTIMATOR_HPP_

#include <ControlTask.hpp>

class AttitudeEstimator : public ControlTask<void> {
   public:
    AttitudeEstimator(StateFieldRegistry& registry);
    void execute() override;

   protected:
    /**
     * @brief Inputs collected from ADCSBoxMonitor.
     */
    std::shared_ptr<ReadableStateField<f_vector_t>> sun_vec_body_rd_fp;
    std::shared_ptr<ReadableStateField<bool>> sun_vec_body_rd_valid_fp;
    std::shared_ptr<ReadableStateField<f_vector_t>> mag_vec_body_rd_fp;
    std::shared_ptr<ReadableStateField<f_vector_t>> gyr_vec_body_rd_fp;
    std::shared_ptr<ReadableStateField<f_vector_t>> rwa_rd_fp;
    std::shared_ptr<ReadableStateField<f_vector_t>> r1_vec_eci_fp;
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
    ReadableStateField<bool> in_eclipse_f;
    ReadableStateField<f_vector_t> h_vec_body_f;
    ReadableStateField<f_quat_t> body_to_eci_f;
};

#endif
