#ifndef ATTITUDE_CONTROLLER_HPP_
#define ATTITUDE_CONTROLLER_HPP_

#include "TimedControlTask.hpp"

#include <gnc/attitude_controller.hpp>
#include <lin/core.hpp>

/**
 * @brief Determines adcs actuations given the current adcs mode and estimated
 * parameters.
 */
class AttitudeController : public TimedControlTask<void> {
   public:
    /**
     * @brief Construct a new attitude controller.
     *
     * @param registry
     */
    AttitudeController(StateFieldRegistry &registry);

    /**
     * @brief Update the attitude controllers suggestion for ADCS actuations.
     */
    void execute() override;

   protected:
    // Inputs from the adcs box monitor
    ReadableStateField<lin::Vector3f> *const w_wheels_rd_fp;

    // Inputs from the attitude estimator
    InternalStateField<bool> const *const attitude_estimator_b_valid_fp;
    InternalStateField<lin::Vector3f> const *const b_body_rd_fp;
    ReadableStateField<bool> const *const attitude_estimator_valid_fp;
    ReadableStateField<lin::Vector4f> const *const q_body_eci_est_fp;
    InternalStateField<lin::Vector3f> const *const w_body_est_fp;

    // Inputs from mission manager
    WritableStateField<unsigned char> const *const adcs_state_fp;

    // Inputs from the time estimator
    ReadableStateField<bool> const *const time_valid_fp;
    InternalStateField<double> const *const time_fp;

    // Inputs from the orbit estimator
    ReadableStateField<bool> const *const orbit_valid_fp;
    ReadableStateField<lin::Vector3d> const *const pos_ecef_fp;
    ReadableStateField<lin::Vector3d> const *const vel_ecef_fp;

    // Inputs from the relative orbit estimator
    ReadableStateField<unsigned char> const *const rel_orbit_state_fp;
    ReadableStateField<lin::Vector3d> const *const pos_baseline_ecef_fp;

    Serializer<lin::Vector3f> unit_vector_sr;
    // Inputs/intermediate pointing objective states
    ReadableStateField<lin::Vector3f> pointer_vec1_current_f;
    ReadableStateField<lin::Vector3f> pointer_vec2_current_f;
    // writable to allow ground override
    WritableStateField<lin::Vector3f> pointer_vec1_desired_f;
    WritableStateField<lin::Vector3f> pointer_vec2_desired_f;

    // Output actuator suggestions, set to writable to allow ground override
    // but technically should never be ground override, instead override the cmder fields
    WritableStateField<lin::Vector3f> t_body_cmd_f;
    WritableStateField<lin::Vector3f> m_body_cmd_f;

    // Structs for GNC detumbler controller
    gnc::DetumbleControllerState detumbler_state;
    gnc::DetumbleControllerData detumbler_data;
    gnc::DetumbleActuation detumbler_actuation;

    // Structs for GNC pointing controller
    gnc::PointingControllerState pointer_state;
    gnc::PointingControllerData pointer_data;
    gnc::PointingActuation pointer_actuation;

    /**
     * @brief Default acutator commands to zeros
     * 
     * We set these state fields to zeros as opposed to NaNs by default so we
     * command zero torque on the spacecraft if something fails.
     */
    void default_actuator_commands();

    /**
     * @brief Default pointing objectives to nans
     * 
     */
    void default_pointing_objectives();
    
    /**
     * @brief Makes a call to gnc::detumble_controller.
     * 
     * Uses all 3 gnc::Detumble structs
     */
    void calculate_detumble_controller();

    /**
     * @brief Calculates a DCM and dr_body if it can.
     * Then based on our mode we calculate the pointing vector current states and objectives.
     * 
     * If we are not in either standby or docking, 
     * no pointing strategy is defined.
     * 
     */
    void calculate_pointing_objectives();

    /**
     * @brief Given the pointing vector current states and objectives,
     * calculate the desired wheel commands and magnetorquer commands
     * 
     * Uses all 3 gnc::Pointing structs
     */
    void calculate_pointing_controller();
};
#endif
