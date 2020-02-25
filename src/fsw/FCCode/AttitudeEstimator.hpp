#ifndef ATTITUDE_ESTIMATOR_HPP_
#define ATTITUDE_ESTIMATOR_HPP_

#include <gnc_attitude_estimation.hpp>
#include <lin.hpp>
#include "TimedControlTask.hpp"

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
     * @brief This is called from MCL to find the radio state after QM has be constructed.
     * 
     */
    void init();
    
    /**
     * @brief Using raw sensor inputs, determine the attitude and angular state
     * of the spacecraft.
     */
    void execute() override;

   protected:

    /**
     * @brief Read data from field pointers and set the data object. 
     * If quake is transcieving and deaf is active, data is set to nan.
     * 
     */
    void set_data();

    /**
     * @brief Set the estimate object from the outputs of the gnc estimate
     * 
     */
    void set_estimate();
    
    static const gps_time_t pan_epoch;

    /**
     * @brief Inputs collected from Piksi and ADCSBoxMonitor.
     */
    //! Time from Piksi
    const ReadableStateField<gps_time_t>* piksi_time_fp;
    //! Position of this satellite, Vector of doubles
    const ReadableStateField<d_vector_t>* pos_vec_ecef_fp;
    //! Sun vector of this satellite, in the body frame.
    const ReadableStateField<f_vector_t>* ssa_vec_rd_fp;
    //! Magnetic field vector of this satellite in the body frame.
    const ReadableStateField<f_vector_t>* mag_vec_fp;

    //kyle's gnc structs
    gnc::AttitudeEstimatorData data;
    gnc::AttitudeEstimatorState state;
    gnc::AttitudeEstimate estimate;

    //AttitudeEstimate
    // Quaternion that converts from the ECI frame to the body frame
    ReadableStateField<f_quat_t> q_body_eci_f;
    // Angular velocity of spacecraft in body frame
    ReadableStateField<f_vector_t> w_body_f;
    // Angular momentum of spacecraft in body frame
    InternalStateField<lin::Vector3f> h_body_f;

    // True if the "paired" ADCS gains are being used.
    WritableStateField<bool> adcs_paired_f;

    // Pointer to the radio state
    const InternalStateField<unsigned char>* radio_state_fp;
    // Ground commandablefield that toggles whether or not 
    // AttitudeEstimator is deaf to piksi data during transcieve
    WritableStateField<bool> data_deaf_f;
};

#endif
