#ifndef MISSION_MANAGER_HPP_
#define MISSION_MANAGER_HPP_

#include <ControlTask.hpp>
#include <StateField.hpp>
#include <memory>

#include "mission_mode_t.enum"

class MissionManager : public ControlTask<void> {
   public:
    MissionManager(StateFieldRegistry& registry);
    void execute() override;

   protected:
    void dispatch_detumble();
    std::shared_ptr<WritableStateField<unsigned int>> adcs_mode_fp;
    std::shared_ptr<WritableStateField<std::array<float, 3>>> adcs_cmd_attitude_fp;

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

    Serializer<unsigned int> mission_mode_serializer;
    WritableStateField<unsigned int> mission_mode_f;
    Serializer<bool> is_deployed_serializer;
    ReadableStateField<bool> is_deployed_f;
};

#endif
