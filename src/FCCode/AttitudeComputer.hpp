#ifndef ATTITUDE_COMPUTER_HPP_
#define ATTITUDE_COMPUTER_HPP_

#include <ControlTask.hpp>
#include "adcs_mode_t.enum"

/**
 * @brief Using the estimator outputs and the current
 * high-level pointing strategy, output the quaternion representing
 * the commanded attitude, and the torques/RWA speeds/magnetorquer
 * commands required to get to this attitude.
 */
class AttitudeComputer : public ControlTask<void> {
   public:
    /**
     * @brief Construct a new Attitude Computer.
     * 
     * @param registry 
     */
    AttitudeComputer(StateFieldRegistry& registry);

    /**
     * @brief Using the estimator outputs and the current
     * high-level pointing strategy, output the quaternion representing
     * the commanded attitude, and the torques/RWA speeds/magnetorquer
     * commands required to get to this attitude.
     */
    void execute() override;

   protected:
    // Dispatch functions for the individual states of the ADCS state machine.
    void dispatch_startup();
    void dispatch_limited();
    void dispatch_zero_torque();
    void dispatch_detumble();
    void dispatch_point_manual();
    void dispatch_point_standby();
    void dispatch_point_docking();

    /**
     * @brief Inputs, a.k.a estimator outputs.
     */
    //! True if the satellite is in eclipse.
    std::shared_ptr<ReadableStateField<bool>> in_eclipse_f;
    //! Spacecraft angular momentum vector, in the body frame.
    std::shared_ptr<ReadableStateField<f_vector_t>> h_vec_body_f;
    //! Quaternion representing the rotation from the body frame to the ECI frame.
    std::shared_ptr<ReadableStateField<f_quat_t>> body_to_eci_f;

    /**
     * @brief Attitude mode: determines the high-level
     * pointing strategy of the spacecraft. See adcs_mode_t.enum.
     */
    WritableStateField<unsigned int> adcs_mode_f;

    /**
     * @brief Computer outputs.
     */
    //! Desired attitude.
    ReadableStateField<f_quat_t> cmd_attitude_f;
    //! True if the next command to apply on the reaction wheels is a torque command
    //! false if it should be a speed command. 
    ReadableStateField<bool> rwa_cmd_mode_f;
    //! Reaction wheel torques to apply on the next timestep in order to start
    //! reaching the desired attitude.
    ReadableStateField<f_vector_t> rwa_torque_cmd_vec_f;
    //! Reaction wheel speeds to apply on the next timestep in order to start
    //! reaching the desired attitude.
    ReadableStateField<f_vector_t> rwa_speed_cmd_vec_f;
    //! Magnetorquer moment commands to apply on the next timestep in order to start
    //! reaching the desired attitude.
    ReadableStateField<f_vector_t> mtr_cmd_vec_f;

    /**
     * @brief Fault flags.
     */
    //! True if the most recently computed commanded attitude is within
    //! validity bounds.
    ReadableStateField<bool> cmd_attitude_valid_f;
};

#endif
