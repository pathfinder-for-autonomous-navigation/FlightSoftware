from .mission import DualSatFarFieldCase
from .utils import Enums, TestCaseFailure


class PiksiFaultFarField(DualSatFarFieldCase):
   def post_boot(self):
      super(PiksiFaultFarField, self).post_boot()

      """We need to enable the piksi fault handler again and call far field's post_boot function.
      """
      self.flight_controller_leader.write_state("fault_handler.enabled", True)
      self.flight_controller_follower.write_state("fault_handler.enabled", True)

      self.flight_controller_leader.write_state("piksi_fh.enabled", True)
      self.flight_controller_follower.write_state("piksi_fh.enabled", True)

      self.flight_controller_leader.write_state("piski_fh.dead.suppress", False)
      self.flight_controller_follower.write_state("piski_fh.dead.suppress", False)

      self.piksi_dead_threshold = self.leader_one_day_ccno//6 + 1
      self.debug_to_console = True      
      self.mock_sensors = False

   def check_no_fix(self):
      if self.flight_controller_leader.smart_read("piksi.state") == Enums.piksi_modes["fixed_rtk"]:
         raise TestCaseFailure("Leader Piksi mode is still 'fixed_rtk'")
      if self.flight_controller_follower.smart_read("piksi.state") == Enums.piksi_modes["fixed_rtk"]:
         raise TestCaseFailure("Follower Piksi mode is still 'fixed_rtk'")

   def check_is_standby(self):
      if self.mission_state_leader != "standby":
         raise TestCaseFailure("Failed to set leader mission state to 'standby'. Mission state is: " + str(self.mission_state_leader))
      # if self.mission_state_follower != "standby":
      #    raise TestCaseFailure("Failed to set follower mission state to 'standby'. Mission state is: " + str(self.mission_state_follower))

   def check_piksi_dead_fault(self, mission_state="leader", error="crc_error", status=None):
      self.mission_state_leader = mission_state
     
      self.cycle()
      self.collect_diagnostic_data()

      # Cycle for time until fault would be triggered
      for i in range(self.piksi_dead_threshold):
         print(i)       # Debugging
         if self.mission_state_leader != mission_state:
            raise TestCaseFailure("Failed to set mission state to " + mission_state + ". Mission state is " + self.mission_state_leader + ".")
         self.flight_controller_leader.write_state("piksi.state", Enums.piksi_modes[error])
         print(self.flight_controller_leader.read_state("piksi.state"))

         self.cycle()
         self.collect_diagnostic_data()

      if status=="should_fault":
         self.check_is_standby()
      else:
         if self.mission_state_leader == "standby":
            raise TestCaseFailure("Piksi dead fault incorrectly set mission state to 'standby'.")

   def collect_diagnostic_data(self):
      self.flight_controller_leader.smart_read("piksi.state")
      self.flight_controller_leader.smart_read("pan.state")
      self.flight_controller_leader.smart_read("qfh.state")
      self.flight_controller_leader.smart_read("prop.state")
      # self.flight_controller_leader.smart_read("prop.overpressured.base")
      # self.flight_controller_leader.smart_read("prop.tank2_temp_high.base")
      # self.flight_controller_leader.smart_read("prop.tank1_temp_high.base")

   
   def run(self):
      """
      Tests piksi dead fault conditions.
      """
      self.logger.put("Running piksi fault handler test: Piksi is Dead")

      print(self.flight_controller_leader.smart_read("piksi.state"))
      print(self.mission_state_leader)

      self.check_piksi_dead_fault(error="crc_error", status="should_fault")

      # Trying to reset piksi mode?
      # self.flight_controller_leader.write_state("piksi.state", Enums.piksi_modes["fixed_rtk"])
      # self.cycle()
      # self.collect_diagnostic_data()
      # self.cycle()
      # self.collect_diagnostic_data()


      self.check_piksi_dead_fault(error="no_data_error", status="should_fault")

      # Trying to reset piksi mode?
      # self.flight_controller_leader.write_state("piksi.state", Enums.piksi_modes["fixed_rtk"])
      # self.cycle()
      # self.collect_diagnostic_data()
      # self.cycle()
      # self.collect_diagnostic_data()

      self.check_piksi_dead_fault(error="data_error", status="should_fault")

      # self.check_piksi_dead_fault(mission_state="startup", error="crc_fault")
      # self.check_piksi_dead_fault(mission_state="detumble", error="crc_fault")
      # self.check_piksi_dead_fault(mission_state="initialization_hold", error="crc_fault")

      self.finish()