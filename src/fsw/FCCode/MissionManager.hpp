#ifndef MISSION_MANAGER_HPP_
#define MISSION_MANAGER_HPP_

#include "TimedControlTask.hpp"
#include "constants.hpp"
#include <lin.hpp>

#include "prop_state_t.enum"
#include "mission_state_t.enum"
#include "adcs_state_t.enum"
#include "radio_state_t.enum"
#include "sat_designation_t.enum"

class MissionManager : public TimedControlTask<void> {
   public:
    MissionManager(StateFieldRegistry& registry, unsigned int offset);
    void execute() override;

    // Constants that drive state transitions.
    WritableStateField<double> detumble_safety_factor_f;
    WritableStateField<double> close_approach_trigger_dist_f; // Meters
    WritableStateField<double> docking_trigger_dist_f; // Meters

    /**
     * @brief Number of control cycles to wait during the post-deployment
     * do-nothing period.
     */
    #ifdef FLIGHT
        static constexpr unsigned int deployment_wait = 15000; // ~30 mins
    #else
        static constexpr unsigned int deployment_wait = 100;
    #endif
    /**
     * @brief Number of control cycles to wait before declaring "too long since comms".
     */
    WritableStateField<unsigned int> max_radio_silence_duration_f;

   protected:
    /**
     * @brief Returns true if there are hardware faults on the spacecraft.
     */
    bool check_hardware_faults();

    /**
     * @brief Handles logic while within a state.
     */
    void dispatch_startup();
    void dispatch_detumble();
    void dispatch_initialization_hold();
    void dispatch_standby();
    void dispatch_follower();
    void dispatch_leader();
    void dispatch_follower_close_approach();
    void dispatch_leader_close_approach();
    void dispatch_docking();
    void dispatch_docked();
    void dispatch_safehold();

    /**
     * @brief Handles state transitions that happen upon subsystem assertions.
     */
    void transition_to_state(mission_state_t mission_state,
        adcs_state_t adcs_state,
        prop_state_t prop_state);
    void transition_to_state(mission_state_t mission_state,
        adcs_state_t adcs_state);

    /**
     * @brief Allow spacecraft to be commandeered completely by test software or
     * the ground. This state should almost never be used by the ground.
     * 
     * The only additional control it provides beyond the standby mode is
     * the ability to control attitude, which is a dangerous capability.
     * If this capability is desired, a better way to use it would be via
     * the Attitude Computer's point_manual mode.
     */
    void dispatch_manual();

    // Fields required for control of prop subsystem.
    ReadableStateField<unsigned char>* prop_state_fp;

    // Fields required for control of ADCS subsystem.
    /**
     * @brief Mode of ADCS system.
     **/
    WritableStateField<unsigned char> adcs_state_f;
    /**
     * @brief Current angular momentum of ADCS system in the body frame.
     **/
    InternalStateField<lin::Vector3f>* adcs_ang_momentum_fp;
    /**
     * @brief Field created by AttitudeEstimator that tracks if the satellites
     * should be treated as paired when computing attitude-related variables.
     */
    WritableStateField<bool>* adcs_paired_fp;

    // Fields provided by Piksi and orbital estimation subsystems
    const ReadableStateField<unsigned char>* piksi_mode_fp; // Piksi reading state (fixed RTK, float RTK, SPP, or error state)
    const ReadableStateField<d_vector_t>* propagated_baseline_pos_fp; // Propagated baseline position

    // Information from docking subsystem
    WritableStateField<bool> docking_config_cmd_f;
    const ReadableStateField<bool>* docked_fp;

    /**
     * @brief Radio's mode.
     **/
    InternalStateField<unsigned char>* radio_state_fp;
    InternalStateField<unsigned int>* last_checkin_cycle_fp;

    // Fields that control overall mission state.
    /**
     * @brief Current mission mode (see mission_mode_t.enum)
     */
    WritableStateField<unsigned char> mission_state_f;
    /**
     * @brief True if the satellite has exited the deployment timing phase.
     */
    ReadableStateField<bool> is_deployed_f;
    ReadableStateField<unsigned int> deployment_wait_elapsed_f;

    /**
     * @brief 2 if the satellite is the follower satellite. 1 if the
     * satellite is the leader satellite. 0 if the follower/leader designation
     * hasn't been made yet.
     */
    WritableStateField<unsigned char> sat_designation_f;

   private:
    /**
     * @brief Computes magnitude of baseline position vector.
     */
    double distance_to_other_sat() const;
    bool too_long_since_last_comms() const;

    void set(mission_state_t state);
    void set(adcs_state_t state);
    void set(prop_state_t state);
    void set(radio_state_t state);
    void set(sat_designation_t designation);
};

#endif
