#ifndef ATTITUDE_COMPUTER_HPP_
#define ATTITUDE_COMPUTER_HPP_

#include "TimedControlTask.hpp"
#include <common/constant_tracker.hpp>

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
    const ReadableStateField<lin::Vector4f>* q_body_eci_fp;
    const ReadableStateField<lin::Vector3f>* ssa_vec_fp;

    /**
     * @brief Inputs required from orbit estimator for pointing.
     */
    const ReadableStateField<lin::Vector3d>* pos_fp;
    const ReadableStateField<lin::Vector3d>* baseline_pos_fp;

    /**
     * @brief Fields used in control of attitude. These vectors are
     * all in the body frame.
     */
    WritableStateField<lin::Vector3f> adcs_vec1_current_f;
    WritableStateField<lin::Vector3f> adcs_vec1_desired_f;
    WritableStateField<lin::Vector3f> adcs_vec2_current_f;
    WritableStateField<lin::Vector3f> adcs_vec2_desired_f;

    /**
     * @brief Threshold at which the secondary pointing objective in 
     * standby pointing with GPS data is ignored.
     */
    TRACKED_CONSTANT_SC(float, alignment_threshold, 0.01);
};

#endif
