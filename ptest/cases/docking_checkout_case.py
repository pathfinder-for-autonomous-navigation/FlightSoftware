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
      docking_config_cmd = self.write_state("docksys.config_cmd", "false")
      docked = self.read_state("docksys.docked")
      is_turning = self.read_state("docksys.is_turning")
      while is_turning:
        docked = self.read_state("docksys.docked")
        is_turning = self.read_state("docksys.is_turning")
      if(not docked):
        self.logger.put("Successfully finished undocking config command")
      else:  
        self.logger.put("Undocking attempt unsuccessful")

    def dock(self):
      docking_config_cmd = self.write_state("docksys.config_cmd", "true")
      docked = self.read_state("docksys.docked")
      is_turning = self.read_state("docksys.is_turning")
      while not docked and is_turning:
        docked = self.read_state("docksys.docked")
        is_turning = self.read_state("docksys.is_turning")
      if(docked):
        self.logger.put("Successfully finished docking command")
      else:  
        self.logger.put("Docking attempt unsuccessful")

    def run_case_singlesat(self):
        self.sim.cycle_no = self.sim.flight_controller.read_state(
            "pan.cycle_no")

        # initially, docksys is in the docking configuration but not docked 
        # and is docking. 
        assert(self.str_to_bool(self.read_state("docksys.dock_config")))
        assert(self.str_to_bool(self.read_state("docksys.docked")) == False)
        assert(self.str_to_bool(self.read_state("docksys.config_cmd")) == True)

        # read and check initial speed is what we expect
        assert(abs( float(self.read_state("docksys.step_angle")) - 0.032 ) < 0.0001)
        assert(int(self.read_state("docksys.step_delay")) == 4000)

        #test switch config to undock (rotate 180)
        self.undock()
        # tell the system to dock 
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
