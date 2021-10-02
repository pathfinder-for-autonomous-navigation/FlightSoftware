from .base import DualSatCase, PSimCase
from .utils import Enums, TestCaseFailure

class FullMissionCase(DualSatCase, PSimCase):

  firstStartup = True
  firstDetumble = True
  firstStandby = True
  firstFarField = True
  firstNearField = True

  def __init__(self, *args, **kwargs):
    super(FullMissionCase, self).__init__(*args, **kwargs)
    self.leader_skip_deployment_wait = False
    self.follower_skip_deployment_wait = False
    self.psim_configs += ["truth/deployment"]
    
  def post_boot(self):
    """
      This prints whether the piksi_off is valid after booting
    """
    leader_piksi_off = self.leader.print_rs("gomspace.piksi_off")
    self.soft_assert( leader_piksi_off == True, f"Leader Piksi_off is {leader_piksi_off} when it should be True")
    follower_piksi_off = self.follower.print_rs("gomspace.piksi_off")
    self.soft_assert( follower_piksi_off == True, f"Follower Piksi_off is {follower_piksi_off} when it should be True")

  def run(self):
    
    # During each run, run different actions based on the current follower and leader states
    self.full_mission_checkout(self.follower.rs("pan.state"), self.leader.rs("pan.state"))
    self.cycle()

    
  def full_mission_checkout(self, followerState, leaderState):
    
    # 1. Startup
    if followerState == 0 and leaderState == 0: 
      if self.firstStartup:
        self.logger.put("Follower and Leader Starting Up")
        self.firstStartup = False

    # 2. Detumble
    elif followerState == 1 and leaderState == 1:
      if self.firstDetumble:
        self.logger.put("Follower and Leader Detumbling")
        self.firstDetumble = False

    # 3. Standby
    elif followerState == 3 and leaderState == 3:
      if self.firstStandby:
        self.logger.put("Follower and Leader in Standby")
        self.firstStandby = False
      else:

        # 3.1 After first cycle of standby, check we can disable the radios

        self.logger.put("SETTING FOLLOWER RADIO TO DISABLED:")
        self.follower.print_ws("radio.state", 5)
        self.cycle()
        follower_radio_state = self.follower.print_rs("radio.state")
        self.soft_assert( follower_radio_state == 5, f"Follower Radio State is {follower_radio_state} when it should be 5")
        
        self.logger.put("SETTING LEADER RADIO TO DISABLED:")
        self.leader.print_ws("radio.state", 5)
        self.cycle()
        leader_radio_state = self.leader.print_rs("radio.state")
        self.soft_assert( leader_radio_state == 5, f"Leader Radio State is {leader_radio_state} when it should be 5")

        # 3.2 Set the sattelite designations to trigger Far Field

        self.logger.put("Setting follower sat designation to trigger Far Field / Follower State.")
        self.flight_controller_follower.write_state("pan.sat_designation", Enums.sat_designations["follower"])
        self.logger.put("Setting leader sat designation to trigger Far Field / Leader State.")
        self.flight_controller_leader.write_state("pan.sat_designation", Enums.sat_designations["leader"])


    # 4. Far Field
    elif followerState == 4 and leaderState == 5:
      if self.firstFarField:
        self.logger.put("Follower in Far Field / Follower State")
        self.logger.put("Leader in Far Field / Leader State")
        self.firstFarField = False

    # 5. Near Field
    elif followerState == 6 and leaderState == 7:
      if self.firstNearField:
        self.logger.put("Follower in Near Field / Follower Close Approach State")
        self.logger.put("Leader in Near Field / Leader Close Approach State")
        self.firstNearField = False

        self.cycle()
        
        # 5.1 Test setting the radios back to enabled (or the wait state)

        self.logger.put("SETTING FOLLOWER BACK TO ENABLED (wait):")
        self.follower.print_ws("radio.state",0)
        self.cycle()
        follower_radio_state = self.follower.print_rs("radio.state")
        self.soft_assert( follower_radio_state == 0, f"Follower Radio State is {follower_radio_state} when it should be 0")

        self.logger.put("SETTING LEADER BACK TO ENABLED (wait):")
        self.leader.print_ws("radio.state",0)
        self.cycle()
        leader_radio_state = self.leader.print_rs("radio.state")
        self.soft_assert( leader_radio_state == 0, f"Leader Radio State is {leader_radio_state} when it should be 0")
        
        # If it reaches this point everything worked as expected
        self.finish(error=True)
