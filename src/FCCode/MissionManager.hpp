#ifndef MISSION_MANAGER_HPP_
#define MISSION_MANAGER_HPP_

#include <TimedControlTask.hpp>

#include "mission_mode_t.enum"
#include "adcs_mode_t.enum"
#include "satellite_designation_t.enum"

class MissionManager : public TimedControlTask<void> {
   public:
    MissionManager(StateFieldRegistry& registry, unsigned int offset);
    void execute() override;

   protected:
    void dispatch_startup();
    void dispatch_detumble();
    void dispatch_initialization_hold();
    void dispatch_follower();
    void dispatch_follower_close_approach();
    void dispatch_standby();
    void dispatch_leader_close_approach();
    void dispatch_docking();
    void dispatch_docked();
    void dispatch_paired();
    void dispatch_spacejunk();
    void dispatch_safehold();

    // Fields required for control of ADCS subsystem.
    /**
     * @brief Mode of ADCS system.
     **/
    std::shared_ptr<WritableStateField<unsigned char>> adcs_mode_fp;
    /**
     * @brief Currently commanded attitude of ADCS system.
     **/
    std::shared_ptr<WritableStateField<f_quat_t>> adcs_cmd_attitude_fp;
    /**
     * @brief Current angular rate of ADCS system in the body frame.
     **/
    std::shared_ptr<ReadableStateField<float>> adcs_ang_rate_fp;
    /**
     * @brief Minimum angular rate of ADCS system that can be considered "stable".
     **/
    std::shared_ptr<WritableStateField<float>> adcs_min_stable_ang_rate_fp;

    // Fields that control overall mission state.
    /**
     * @brief Control cycle count, provided by ClockManager.
     */
    std::shared_ptr<ReadableStateField<unsigned int>> control_cycle_count_fp;
    /**
     * @brief Current mission mode (see mission_mode_t.enum)
     */
    Serializer<unsigned char> mission_mode_sr;
    WritableStateField<unsigned char> mission_mode_f;
    /**
     * @brief True if the satellite has exited the deployment timing phase.
     */
    Serializer<bool> is_deployed_sr;
    ReadableStateField<bool> is_deployed_f;
    /**
     * @brief 2 if the satellite is the follower satellite. 1 if the
     * satellite is the leader satellite. 0 if the follower/leader designation
     * hasn't been made yet.
     */
    Serializer<unsigned char> sat_designation_sr;
    WritableStateField<unsigned char> sat_designation_f;
    
};

#endif
