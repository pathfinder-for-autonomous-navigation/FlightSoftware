#ifndef ATTITUDE_ESTIMATOR_HPP_
#define ATTITUDE_ESTIMATOR_HPP_

#include <gnc_attitude_estimation.hpp>
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

   protected:

    /**
     * @brief Using raw sensor inputs, determine the attitude and angular state
     * of the spacecraft.
     */
    void execute() override;

    /**
     * @brief Read data from field pointers and set the data object
     * 
     */
    void set_data();

    /**
     * @brief Set the estimate object from the outputs of the gnc estimate
     * 
     */
    void set_estimate();
    
    const gps_time_t pan_epoch;

    /**
     * @brief Inputs collected from Piksi and ADCSBoxMonitor.
     */
    //! Time from Piksi
    ReadableStateField<gps_time_t>* const piksi_time_fp;
    //! Position of this satellite, Vector of doubles
    ReadableStateField<d_vector_t>* const pos_vec_ecef_fp;
    //! Sun vector of this satellite, in the body frame.
    ReadableStateField<f_vector_t>* const ssa_vec_rd_fp;
    //! Magnetic field vector of this satellite in the body frame.
    ReadableStateField<f_vector_t>* const mag_vec_fp;

    //kyle's gnc structs
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
