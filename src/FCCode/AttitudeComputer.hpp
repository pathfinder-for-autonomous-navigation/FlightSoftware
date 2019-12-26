#ifndef ATTITUDE_COMPUTER_HPP_
#define ATTITUDE_COMPUTER_HPP_

#include "TimedControlTask.hpp"

/**
 * @brief Using raw sensor inputs, determine the attitude and angular state
 * of the spacecraft.
 */
class AttitudeComputer : public TimedControlTask<void> {
   public:
    /**
     * @brief Construct a new Attitude Estimator.
     * 
     * @param registry 
     */
    AttitudeComputer(StateFieldRegistry& registry, unsigned int offset);

    /**
     * @brief Determine control outputs for attitude computer.
     */
    void execute() override;

   protected:
    /**
     * @brief Input required from mission manager.
     */
    const WritableStateField<unsigned char>* adcs_state_fp;

    /**
     * @brief Inputs required from ADCS system for pointing.
     */
    const ReadableStateField<f_quat_t>* q_body_eci_fp;
    const ReadableStateField<f_vector_t>* ssa_vec_fp;

    /**
     * @brief Inputs required from Piksi for pointing.
     */
    const ReadableStateField<gps_time_t>* time_fp;
    const ReadableStateField<d_vector_t>* pos_fp;
    const ReadableStateField<d_vector_t>* baseline_pos_fp;

    /**
     * @brief Fields used in control of attitude.
     */
    WritableStateField<f_vector_t> adcs_vec1_current_f;
    WritableStateField<f_vector_t> adcs_vec1_desired_f;
    WritableStateField<f_vector_t> adcs_vec2_current_f;
    WritableStateField<f_vector_t> adcs_vec2_desired_f;
};

#endif
