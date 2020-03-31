# Docking test case. Gets cycle count purely for diagnostic purposes and 
# tests that the motor rotates 180 degrees with the initial speed and angle
# constants and with changes to the fields from ground.

from .base import SingleSatOnlyCase

class DockingCheckoutCase(SingleSatOnlyCase):

    def setup_case_singlesat(self):
        self.sim.flight_controller.write_state(
            "pan.state", self.mission_states.get_by_name("manual"))

    def str_to_bool(self, str):
        if str == "true":
            return True
        return False

    def read_state(self, string_state):
        return self.sim.flight_controller.read_state(string_state)

    def write_state(self, string_state, state_value):
        self.sim.flight_controller.write_state(string_state, state_value)
        return self.read_state(string_state)

    def undock(self):
      self.logger.put("Starting to undock.")
      docking_config_cmd = self.write_state("docksys.config_cmd", "false")
      dock_config = self.str_to_bool(self.read_state("docksys.dock_config"))
      while dock_config:
        self.write_state("cycle.start", "true")
        cycle_no = int(self.read_state("pan.cycle_no"))
        self.write_state("pan.cycle_no", cycle_no + 1)
        self.logger.put(str(cycle_no))
        dock_config = self.read_state("docksys.dock_config")
        self.logger.put("dock_config: " + dock_config + "\tturning: " + self.read_state("docksys.is_turning") + "\tdocked: " + self.read_state("docksys.docked") + "\tdockcmd: " + self.read_state("docksys.config_cmd"))
      if(not dock_config):
        self.logger.put("Successfully finished undocking config command")
      else:  
        self.logger.put("Undocking attempt unsuccessful")

    def dock(self):
      self.logger.put("Starting to dock.")
      docking_config_cmd = self.write_state("docksys.config_cmd", "true")
      dock_config = self.str_to_bool(self.read_state("docksys.dock_config"))
      while not dock_config:
        self.write_state("cycle.start", "true")
        cycle_no = int(self.read_state("pan.cycle_no"))
        self.write_state("pan.cycle_no", cycle_no + 1)
        self.logger.put(str(cycle_no))
        dock_config = self.read_state("docksys.dock_config")
        self.logger.put("dock_config: " + dock_config + "\tturning: " + self.read_state("docksys.is_turning") + "\tdocked: " + self.read_state("docksys.docked") + "\tdockcmd: " + self.read_state("docksys.config_cmd"))
      if(dock_config):
        self.logger.put("Successfully finished docking command")
      else:  
        self.logger.put("Docking attempt unsuccessful")

    def run_case_singlesat(self):
        self.sim.cycle_no = self.sim.flight_controller.read_state(
            "pan.cycle_no")
        self.sim.flight_controller.write_state("adcs.state", 5) # ADCS State = Manual
        self.write_state("dcdc.SpikeDock_cmd", "true")
        self.write_state("dcdc.ADCSMotor_cmd", "true")
        # initially, docksys is in the docking configuration but not docked, not turning 
        # and is docking. 
        assert(self.str_to_bool(self.read_state("docksys.dock_config")))
        assert(self.str_to_bool(self.read_state("docksys.docked")) == False)
        assert(self.str_to_bool(self.read_state("docksys.is_turning")) == False)
        assert(self.str_to_bool(self.read_state("docksys.config_cmd")))

        # read and check initial speed is what we expect
        assert(abs( float(self.read_state("docksys.step_angle")) - 0.032 ) < 0.0001)
        assert(int(self.read_state("docksys.step_delay")) == 4000)

        #test switch config to undock (rotate 180)
        self.undock()
        # tell the system to go to docking config 
        self.dock()

        # write different step angle and delay and repeat docking cycle
        # testing data shows step angle is around 180/5000 = 0.036, but 
        # we do not have the theoretical value and it may change with different
        # loading conditions.
        self.write_state("docksys.step_angle", "0.036") #initial is .032
        self.write_state("docksys.step_delay", "2000") #initial is 4000

        self.undock()
        self.dock()

        self.finish()
