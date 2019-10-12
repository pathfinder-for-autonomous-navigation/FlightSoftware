#ifndef MISSION_MANAGER_HPP_
#define MISSION_MANAGER_HPP_

#include <ControlTask.hpp>

#include "mission_mode_t.enum"

class MissionManager : public ControlTask<void> {
   public:
    MissionManager(StateFieldRegistry& registry);
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
    std::shared_ptr<WritableStateField<unsigned int>> adcs_mode_fp;
    std::shared_ptr<WritableStateField<f_quat_t>> adcs_cmd_attitude_fp;

    // Fields that control overall mission state.
    Serializer<unsigned int> mission_mode_sr;
    WritableStateField<unsigned int> mission_mode_f;
    Serializer<bool> is_deployed_sr;
    ReadableStateField<bool> is_deployed_f;
    std::shared_ptr<ReadableStateField<unsigned int>> control_cycle_count_fp;
};

#endif
