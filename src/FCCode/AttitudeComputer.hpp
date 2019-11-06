#ifndef ATTITUDE_COMPUTER_HPP_
#define ATTITUDE_COMPUTER_HPP_

#include <ControlTask.hpp>
#include "adcs_mode_t.enum"

class AttitudeComputer : public ControlTask<void> {
   public:
    AttitudeComputer(StateFieldRegistry& registry);

    /**
     * @brief Using the estimator outputs and the current
     * high-level pointing strategy, output the quaternion representing
     * the commanded attitude, and the torques/RWA speeds/magnetorquer
     * commands required to get to this attitude.
     */
    void execute() override;

   protected:
    /**
     * @brief Inputs, a.k.a estimator outputs.
     */
    ReadableStateField<bool> in_eclipse_f;
    ReadableStateField<f_vector_t> h_vec_body_f;
    ReadableStateField<f_quat_t> body_to_eci_f;

    /**
     * @brief Attitude mode: determines the high-level
     * pointing strategy of the spacecraft.
     */
    WritableStateField<unsigned int> adcs_mode_f;

    /**
     * @brief Computer outputs.
     */
    ReadableStateField<f_quat_t> cmd_attitude_f;
    std::shared_ptr<ReadableStateField<f_vector_t>> rwa_torque_vec_f;
    std::shared_ptr<ReadableStateField<f_vector_t>> rwa_speed_vec_f;
    std::shared_ptr<ReadableStateField<f_vector_t>> mtr_cmd_vec_f;
};

#endif
