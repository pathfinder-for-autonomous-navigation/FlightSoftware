#ifndef ATTITUDE_COMPUTER_HPP_
#define ATTITUDE_COMPUTER_HPP_

#include <ControlTask.hpp>

class AttitudeComputer : public ControlTask<void> {
   public:
    AttitudeComputer(StateFieldRegistry& registry);
    void execute() override;

   protected:
    /**
     * @brief Computer inputs / estimator outputs.
     */
    ReadableStateField<bool> in_eclipse_f;
    ReadableStateField<f_vector_t> h_vec_body_f;
    ReadableStateField<f_quat_t> body_to_eci_f;

    /**
     * @brief Attitude mode.
     */
    WritableStateField<unsigned int> adcs_mode_f;

    /**
     * @brief Computer outputs.
     */
    std::shared_ptr<ReadableStateField<f_vector_t>> rwa_torque_vec_f;
    std::shared_ptr<ReadableStateField<f_vector_t>> rwa_speed_vec_f;
    std::shared_ptr<ReadableStateField<f_vector_t>> mtr_cmd_vec_f;
};

#endif
