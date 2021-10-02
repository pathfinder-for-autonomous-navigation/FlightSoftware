from .base import DualSatCase, PSimCase
from .utils import Enums, TestCaseFailure

class FullMissionCase(DualSatCase, PSimCase):

  def __init__(self, *args, **kwargs):
    super(FullMissionCase, self).__init__(*args, **kwargs)
    self.leader_skip_deployment_wait = True
    self.follower_skip_deployment_wait = True
    self.psim_configs += ["truth/standby"]
    
  def post_boot(self):
    """
      This prints whether the piksi_off is valid after booting
    """
    leader_piksi_off = self.leader.print_rs("gomspace.piksi_off")
    if not leader_piksi_off:
      raise TestCaseFailure(f"Leader Piksi_off is {leader_piksi_off} when it should be True")
    follower_piksi_off = self.follower.print_rs("gomspace.piksi_off")
    if not follower_piksi_off:
      raise TestCaseFailure( f"Follower Piksi_off is {follower_piksi_off} when it should be True")

    # Disable to radio to see if it can reenable by standby
    self.logger.put("SETTING FOLLOWER RADIO TO DISABLED:")
    self.follower.print_ws("radio.state", 5)
    self.logger.put("SETTING LEADER RADIO TO DISABLED:")
    self.leader.print_ws("radio.state", 5)

  def run(self):

    self.logger.put("Follower and Leader Starting Up")

    # wait until detumbling
    while self.follower.rs("pan.state") != Enums.mission_states["startup"] or self.leader.rs("pan.state") != Enums.mission_states["startup"]:
      self.cycle()
    
    self.cycle()
    self.logger.put("Follower and Leader Detumbling")

    #self.leader.ws("pan.state", 3)
    #self.follower.ws("pan.state", 3)
    
    # wait until standby
    while self.follower.rs("pan.state") != Enums.mission_states["standby"] or self.leader.rs("pan.state") != Enums.mission_states["standby"]:
      self.cycle()
    
    self.logger.put("Follower and Leader in Standby")

    self.cycle()
    self.cycle()

    # Check if Radio State is not disabled for leader and follower
    follower_radio_state = self.follower.rs("radio.state")
    if follower_radio_state == Enums.radio_states["disabled"]:
      raise TestCaseFailure(f"Follower Radio State is Disabled")
    else:
      self.logger.put("Follower Radio is Enabled")
    
    leader_radio_state = self.leader.rs("radio.state")
    if leader_radio_state == Enums.radio_states["disabled"]:
       raise TestCaseFailure(f"Leader Radio State is Disabled")
    else:
      self.logger.put("Leader Radio is Enabled")

    # Set satellite designations
    self.logger.put("Setting follower sat designation to trigger Far Field / Follower State.")
    self.flight_controller_follower.write_state("pan.sat_designation", Enums.sat_designations["follower"])
    self.logger.put("Setting leader sat designation to trigger Far Field / Leader State.")
    self.flight_controller_leader.write_state("pan.sat_designation", Enums.sat_designations["leader"])

    # Wait for follower and Leader
    while self.follower.rs("pan.state") != Enums.mission_states["follower"] or self.leader.rs("pan.state") != Enums.mission_states["leader"]:
      self.cycle()

    
    self.logger.put("Follower in Far Field / Follower State")
    self.logger.put("Leader in Far Field / Leader State")

    # Wait for near field
    while self.follower.rs("pan.state") != Enums.mission_states["follower_close_approach"] or self.leader.rs("pan.state") != Enums.mission_states["leader_close_approach"]:
      self.leader.ws("pan.state", Enums.mission_states["leader_close_approach"])
      self.follower.ws("pan.state", Enums.mission_states["follower_close_approach"])
      self.cycle()
    
    self.logger.put("Follower in Near Field / Follower Close Approach State")
    self.logger.put("Leader in Near Field / Leader Close Approach State")

    # If it made it to nearfield with no assertion, then it passed
    self.finish()


  