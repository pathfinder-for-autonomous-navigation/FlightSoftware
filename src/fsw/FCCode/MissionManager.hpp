#ifndef MISSION_MANAGER_HPP_
#define MISSION_MANAGER_HPP_

#include "TimedControlTask.hpp"
#include "QuakeFaultHandler.hpp"
#include "constants.hpp"
#include <lin.hpp>

#include <common/Fault.hpp>
#include "MainFaultHandler.hpp"
#include "prop_state_t.enum"
#include "mission_state_t.enum"
#include "adcs_state_t.enum"
#include "radio_state_t.enum"
#include "sat_designation_t.enum"

class MissionManager : public TimedControlTask<void> {
   #ifdef UNIT_TEST
     friend class TestFixture;
   #endif

   public:
    MissionManager(StateFieldRegistry& registry, unsigned int offset);
    void execute() override;

    // Constants that drive state transitions.
    WritableStateField<double> detumble_safety_factor_f;
    WritableStateField<double> close_approach_trigger_dist_f; // Meters
    WritableStateField<double> docking_trigger_dist_f; // Meters
    TRACKED_CONSTANT_SC(double, initial_detumble_safety_factor, 0.2);
    TRACKED_CONSTANT_SC(double, initial_close_approach_trigger_dist, 100); // Meters
    TRACKED_CONSTANT_SC(double, initial_docking_trigger_dist, 0.4); // Meters

    /**
     * @brief Number of control cycles to wait during the post-deployment
     * do-nothing period.
     */
    #ifdef FLIGHT
        TRACKED_CONSTANT_SC(unsigned int, deployment_wait, 15000); // ~30 mins
    #else
        TRACKED_CONSTANT_SC(unsigned int, deployment_wait, 100);
    #endif
    /**
     * @brief Number of control cycles to wait before declaring "too long since comms".
     */
    WritableStateField<unsigned int> max_radio_silence_duration_f;
    TRACKED_CONSTANT_SC(unsigned int, initial_max_radio_silence_duration, PAN::one_day_ccno);

    /**
     * @brief Number of control cycles to wait while in docking state before moving to standby
     */
    WritableStateField<unsigned int> docking_timeout_limit_f;
    TRACKED_CONSTANT_SC(unsigned int, initial_docking_timeout_limit, PAN::one_day_ccno);

    // These states respond to fault conditions.
    static constexpr std::array<mission_state_t, 5> fault_responsive_states = {
        mission_state_t::follower,
        mission_state_t::leader,
        mission_state_t::follower_close_approach,
        mission_state_t::leader_close_approach,
        mission_state_t::standby
    };
    // These states do not respond to fault conditions.
    static constexpr std::array<mission_state_t, 7> fault_nonresponsive_states = {
        mission_state_t::detumble,
        mission_state_t::safehold,
        mission_state_t::startup,
        mission_state_t::manual,
        mission_state_t::docking,
        mission_state_t::docked,
        mission_state_t::initialization_hold
    };

    void set(mission_state_t state);

   protected:
    /**
     * @brief Returns true if there are hardware faults on the spacecraft.
     * 
     * This function is used in the startup state to determine whether the spacecraft
     * should switch to the detumble or the initialization hold state, after completing
     * its deployment wait.
     * 
     */
    bool check_adcs_hardware_faults() const;

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
    unsigned int safehold_begin_ccno = 0; // Control cycle # of the most recent
                                          // transition to safe hold.

    // Fault handler class.
    std::unique_ptr<FaultHandlerMachine> main_fault_handler;

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
    const ReadableStateField<lin::Vector3d>* propagated_baseline_pos_fp; // Propagated baseline position

    // Field exposed by Gomspace for rebooting entire spacecraft.
    WritableStateField<bool>* reboot_fp;

    // Information from docking subsystem
    WritableStateField<bool> docking_config_cmd_f;
    const ReadableStateField<bool>* docked_fp;
    InternalStateField<unsigned int> enter_docking_cycle_f;

    // True if the battery is below the threshold for safehold.
    Fault* low_batt_fault_fp;
    // Fault flags for ADCS motor ADCs and potentiometer.
    Fault* adcs_functional_fault_fp;
    Fault* wheel1_adc_fault_fp;
    Fault* wheel2_adc_fault_fp;
    Fault* wheel3_adc_fault_fp;
    Fault* wheel_pot_fault_fp;
    // Flag for if propulsion failed to pressurize.
    Fault* failed_pressurize_fp;

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
    bool too_long_in_docking() const;

    void set(adcs_state_t state);
    void set(prop_state_t state);
    void set(radio_state_t state);
    void set(sat_designation_t designation);
};

#endif
