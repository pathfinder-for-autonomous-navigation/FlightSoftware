from .mission import DualSatNearFieldCase
from .utils import Enums, TestCaseFailure


class PiksiFaultNearField2(DualSatNearFieldCase):
   """Test the basic functionality of the piksi fault handler.
   """
   def __init__(self, *args, **kwargs):
      super(PiksiFaultNearField2, self).__init__(*args, **kwargs)
      self.debug_to_console = False

   def post_boot(self):
      """We need to enable the piksi fault handler again and call near field's post_boot function.
      """
      self.flight_controller_leader.write_state("fault_handler.enabled", True)
      self.flight_controller_follower.write_state("fault_handler.enabled", True)
      self.flight_controller_leader.write_state("piksi_fh.enabled", True)
      self.flight_controller_follower.write_state("piksi_fh.enabled", True)

      self.no_cdpgs_max_wait = self.flight_controller_leader.smart_read("piksi_fh.no_cdpgs_max_wait")

      # Shut off cdgps readings
      self.ws_psim("sensors.leader.cdgps.disabled", True)
      self.ws_psim("sensors.follower.cdgps.disabled", True)
      
      for _ in range(10):
         self.cycle()

      self.flight_controller_leader.write_state("pan.sat_designation", Enums.sat_designations["leader"])
      self.flight_controller_follower.write_state("pan.sat_designation", Enums.sat_designations["follower"])
      
      self.cycle()
      self.cycle()
      
      if Enums.rel_orbit_state[self.flight_controller_leader.smart_read("rel_orbit.state")] != "estimating":
         raise TestCaseFailure("The leader's relative orbit estimator should be propagating.")
      if Enums.mission_states[self.flight_controller_leader.smart_read("pan.state")] != "leader_close_approach":
         raise TestCaseFailure("The leader's mission state should be in leader close approach.")

      if Enums.rel_orbit_state[self.flight_controller_follower.smart_read("rel_orbit.state")] != "estimating":
         raise TestCaseFailure("The follower's relative orbit estimator should be propagating.")
      if Enums.mission_states[self.flight_controller_follower.smart_read("pan.state")] != "follower_close_approach":
         raise TestCaseFailure("The follower's mission state should be in follower close approach.")

   def check_no_fix(self):
      if self.flight_controller_leader.smart_read("piksi.state") == Enums.piksi_modes["fixed_rtk"]:
         raise TestCaseFailure("Leader Piksi mode is still 'fixed_rtk'")
      if self.flight_controller_follower.smart_read("piksi.state") == Enums.piksi_modes["fixed_rtk"]:
         raise TestCaseFailure("Follower Piksi mode is still 'fixed_rtk'")

   def check_is_close_appr(self):
      self.mission_state_leader = self.flight_controller_leader.smart_read("pan.state")
      self.mission_state_follower = self.flight_controller_follower.smart_read("pan.state")
      if self.mission_state_leader != Enums.mission_states["leader_close_approach"]:
         raise TestCaseFailure("Failed to set mission state to 'leader_close_approach'. Mission state is: " + str(self.mission_state_leader))
      if self.mission_state_follower != Enums.mission_states["follower_close_approach"]:
         raise TestCaseFailure("Failed to set mission state to 'follower_close_approach'. Mission state is: " + str(self.mission_state_follower))

   def check_is_standby(self):
      self.mission_state_leader = self.flight_controller_leader.smart_read("pan.state")
      self.mission_state_follower = self.flight_controller_follower.smart_read("pan.state")
      if self.mission_state_leader != Enums.mission_states["standby"]:
         raise TestCaseFailure("Failed to set leader mission state to 'standby'. Mission state is: " + str(self.mission_state_leader))
      if self.mission_state_follower != Enums.mission_states["standby"]:
         raise TestCaseFailure("Failed to set follower mission state to 'standby'. Mission state is: " + str(self.mission_state_follower))

   def run(self):
      """If we lose cdgps readings and cycle for an arbitrary number of times
      before entering close_approach, after no_cdpgs_max_wait cycles in 
      close_approach, we transition to standby.
      """
      for i in range(self.no_cdpgs_max_wait + 1):
         self.check_is_close_appr()
         self.check_no_fix()
         self.cycle()

      self.check_is_standby()
      self.logger.put("PiksiFaultHandler successfully changed mission state to 'standby' after waiting for 'np_cdpgs_delay_max_wait' cycles.")
      self.finish()
