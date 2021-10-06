
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

class MissionManager : public TimedControlTask<void>
{
#ifdef UNIT_TEST
    friend class TestFixture;
#endif

public:
    MissionManager(StateFieldRegistry &registry);
    void execute() override;

    // Constants that drive state transitions.
    WritableStateField<double> detumble_safety_factor_f;
    WritableStateField<double> close_approach_trigger_dist_f; // Meters
    WritableStateField<double> docking_trigger_dist_f;        // Meters
    TRACKED_CONSTANT_SC(double, initial_detumble_safety_factor, 0.2);
    TRACKED_CONSTANT_SC(double, initial_close_approach_trigger_dist, 2000); // Meters
    TRACKED_CONSTANT_SC(double, initial_docking_trigger_dist, 0.4);         // Meters

    /**
     * @brief Number of control cycles to wait during the post-deployment
     * do-nothing period. Should be equivalent to 30 minutes.
     */
    TRACKED_CONSTANT_SC(unsigned int, deployment_wait, PAN::one_day_ccno / (24 * 2));

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
        mission_state_t::standby};
    // These states do not respond to fault conditions.
    static constexpr std::array<mission_state_t, 7> fault_nonresponsive_states = {
        mission_state_t::detumble,
        mission_state_t::safehold,
        mission_state_t::startup,
        mission_state_t::manual,
        mission_state_t::docking,
        mission_state_t::docked,
        mission_state_t::initialization_hold};

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
    void transition_to(mission_state_t mission_state,
                       adcs_state_t adcs_state,
                       prop_state_t prop_state);
    void transition_to(mission_state_t mission_state,
                       adcs_state_t adcs_state);

    /**
     * @brief Allow spacecraft to be commandeered completely by test software or
     * the ground. This state should almost never be used by the ground.
     * 
     * The only additional control it provides beyond the standby mode is
     * the ability to control attitude, which is a dangerous capability.
     * If this capability is desired, a better way to use it would be via
     * the AttitudeController's point_manual mode.
     * 
     * Manual mission mode is to be used whenever the adcs_state is point_manual or manual
     */
    void dispatch_manual();

    // Fields required for control of prop subsystem.
    WritableStateField<unsigned int> *prop_state_fp;

    // Fields required for control of ADCS subsystem.
    /**
     * @brief Mode of ADCS system.
     **/
    WritableStateField<unsigned char> adcs_state_f;
    /**
     * @brief Current, estimated angular rate in the body frame (radians per
     *        second).
     * 
     * Inputs from the AttitudedEstimator. */
    ReadableStateField<bool> const *attitude_estimator_valid_fp;
    ReadableStateField<lin::Vector3f> const *attitude_estimator_L_body_fp;

    // Fields provided by relative orbit estiamtor
    const ReadableStateField<unsigned char> *rel_orbit_state_fp;
    const ReadableStateField<lin::Vector3d> *rel_orbit_rel_pos_fp;

    // Field exposed by Gomspace for resetting entire spacecraft.
    WritableStateField<bool> *reset_fp;

    // Information from docking subsystem
    WritableStateField<bool> docking_config_cmd_f;
    const ReadableStateField<bool> *docked_fp;
    InternalStateField<unsigned int> enter_docking_cycle_f;

    // True if the battery is below the threshold for safehold.
    Fault *const low_batt_fault_fp;
    // Fault flags for ADCS motor ADCs and potentiometer.
    Fault *const adcs_functional_fault_fp;
    Fault *const wheel1_adc_fault_fp;
    Fault *const wheel2_adc_fault_fp;
    Fault *const wheel3_adc_fault_fp;
    Fault *const wheel_pot_fault_fp;
    // Flag for if propulsion failed to pressurize.
    Fault *const pressurize_fail_fp;

    /**
     * @brief DCDC control flag for Spike and Hold and docking system.
     */
    WritableStateField<bool> *sph_dcdc_fp;

    /**
     * @brief DCDC control Disables/enables wheels for the ADCS system.
     */
    WritableStateField<bool> *adcs_dcdc_fp;

    /**
     * @brief Radio's mode.
     **/
    ReadableStateField<unsigned char> *radio_state_fp;
    ReadableStateField<unsigned int> *last_checkin_cycle_fp;

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

    /**
     * @brief The cycle at which we enter the close approach state
     */
    InternalStateField<unsigned int> enter_close_approach_ccno_f;

    /**
     * @brief The command to shut down all communication 
     * from spacecraft with ground when set to 127.
     */
    WritableStateField<unsigned char> kill_switch_f;
    TRACKED_CONSTANT_SC(unsigned char, kill_switch_value, 127);

    /**
     * @brief Number of times the satellite has booted
     */
    ReadableStateField<unsigned int> *bootcount_fp;

    /**
     * @brief True if Gomspace is not supplying power to port that Piksi is connected to (OUT-1)
     */
    WritableStateField<bool> *piksi_off_fp;

    /**
     * @brief True if Gomspace should power cycle piksi port
     */
    WritableStateField<bool> *piksi_powercycle_fp;

private:
    /**
     * @brief Computes magnitude of baseline position vector.
     */
    double distance_to_other_sat() const;
    bool too_long_in_docking() const;

    void set(adcs_state_t state);
    void set(prop_state_t state);
    void set(radio_state_t state);
    void set(sat_designation_t designation);
};

#endif
