#ifndef ATTITUDE_CONTROLLER_HPP_
#define ATTITUDE_CONTROLLER_HPP_

#include "TimedControlTask.hpp"

#include <gnc/attitude_controller.hpp>
#include <lin/core/vector/vector.hpp>

/**
 * @brief Determines adcs actuations given the current adcs mode and estimated
 * parameters.
 */
class AttitudeController : public TimedControlTask<void> {
   public:
    /**
     * @brief Construct a new attitude estimator.
     *
     * @param registry
     * @param offset
     */
    AttitudeEstimator(StateFieldRegistry &registry, unsigned int offset);
    ~AttitudeEstimator() = default;

    /**
     * @brief Update the attitude controllers suggestion for ADCS actuations.
     */
    void execute() override;

   protected:
    // Inputs from the adcs box monitor
    ReadableStateField<lin::Vector3f> const *const b_body_rd_fp;

    // Inputs from the attitude estimator
    ReadableStateField<lin::Vector3f> const *const b_body_est_fp;
    ReadableStateField<lin::Vector3f> const *const s_body_est_fp;
    ReadableStateField<lin::Vector4f> const *const q_body_eci_est_fp;
    ReadableStateField<lin::Vector3f> const *const w_body_est_fp;

    // Inputs from mission manager
    WritableStateField<unsigned char> const *const adcs_state_fp;

    // Inputs from the orbit estimator
    ReadableStateField<unsigned long> const *const time_ns;
    ReadableStateField<lin::Vector3d> const *const pos_ecef_fp;
    ReadableStateField<lin::Vector3d> const *const vel_ecef_fp;
    ReadableStateField<lin::Vector3d> const *const pos_baseline_ecef_fp;

    // TODO : Determine where this is from
    ReadableStateField<lin::Vector3f> const *const pan_time_fp;

    // Inputs/intermediate pointing objective states
    ReadableStateField<lin::Vector3f> pointer_vec1_current_f;
    WritableStateField<lin::Vector3f> pointer_vec1_desired_f;
    ReadableStateField<lin::Vector3f> pointer_vec2_current_f;
    WritableStateField<lin::Vector3f> pointer_vec2_desired_f;

    // Output actuator suggestions
    ReadableStateField<lin::Vector3f> t_body_cmd_f;  // TODO : Figure out bounds for this
    ReadableStateField<lin::Vector3f> m_body_cmd_f;  // TODO : Figure out bounds for this

    // Structs for the psim attitude controller adapters
    gnc::DetumbleControllerState detumbler_state;
    gnc::PointingControllerState pointer_state;
    union {
      struct {
        gnc::DetumbleControllerData detumbler_data;
        gnc::DetumbleActuation detumbler_actuation;
      };
      struct {
        gnc::PointingControllerData pointer_data;
        gnc::PointingActuation pointer_actuation;
      };
    };

    void default_actuator_commands();
    void default_pointing_objectives();
    void default_all();
    void calculate_detumble_controller();
    void calculate_pointing_objectives();
    void calculate_pointing_controller();
};
