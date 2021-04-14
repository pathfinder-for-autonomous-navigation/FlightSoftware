# Docking test case. Gets cycle count purely for diagnostic purposes and 
# tests that the motor rotates 180 degrees with the initial speed and angle
# constants and with changes to the fields from ground.

from .base import SingleSatCase
from .utils import Enums

class DockingCheckoutCase(SingleSatCase):
    def str_to_bool(self, str):
        if str == "true":
            return True
        return False

    def read_state(self, string_state):
        return self.flight_controller.read_state(string_state)

    def write_state(self, string_state, state_value):
        self.flight_controller.write_state(string_state, state_value)
        return self.read_state(string_state)

    def log_docking_states(self):
      self.logger.put("Cycle: " + self.read_state("pan.cycle_no") 
                      + "\tdock_config: " + self.read_state("docksys.dock_config") 
                      + "\tturning: " + self.read_state("docksys.is_turning") 
                      + "\tdocked: " + self.read_state("docksys.docked") 
                      + "\tdock_cmd: " + self.read_state("docksys.config_cmd")
                      + "\tbattery_voltage: " + self.read_state("gomspace.vbatt")
                      + "\tbattery_current : " + self.read_state("gomspace.cursys"))

    def undock(self):
      #send command to go into undocked configuration
      self.write_state("docksys.config_cmd", "false")     
      self.logger.put("\nStarting to undock.")
      dock_config = self.str_to_bool(self.read_state("docksys.dock_config"))
      start_cycle = int(self.read_state("pan.cycle_no"))
      self.log_docking_states()
      #wait for undocking to finish
      while dock_config:
        self.cycle()
        dock_config = self.str_to_bool(self.read_state("docksys.dock_config"))
        if int(self.read_state("pan.cycle_no"))%1000 == 0 or not dock_config:
          self.log_docking_states()
      cycles_taken = int(self.read_state("pan.cycle_no")) - start_cycle
      self.logger.put("Successfully finished undocking config command in " + str(cycles_taken) + " cycles\n")

    def dock(self):
      #send command to go into docked configuration
      self.write_state("docksys.config_cmd", "true")
      self.logger.put("\nStarting to dock.")
      dock_config = self.str_to_bool(self.read_state("docksys.dock_config"))
      start_cycle = int(self.read_state("pan.cycle_no"))
      self.log_docking_states()     
      #wait for docking to finish
      while not dock_config:
        self.cycle()
        dock_config = self.str_to_bool(self.read_state("docksys.dock_config"))
        if int(self.read_state("pan.cycle_no"))%1000 == 0 or dock_config:
          self.log_docking_states()
      cycles_taken = int(self.read_state("pan.cycle_no")) - start_cycle
      self.logger.put("Successfully finished docking config command in " + str(cycles_taken) + " cycles\n")

    def run(self):
        self.cycle_no = self.read_state("pan.cycle_no")
        self.write_state("dcdc.SpikeDock_cmd", "true")
        assert(self.read_state("dcdc.SpikeDock_cmd") =="true")

        # initially, docksys is in the docking configuration but not docked, not turning 
        # and is docking. 
        assert(self.str_to_bool(self.read_state("docksys.dock_config")))
        assert(self.str_to_bool(self.read_state("docksys.docked")) == False)
        assert(self.str_to_bool(self.read_state("docksys.is_turning")) == False)
        assert(self.str_to_bool(self.read_state("docksys.config_cmd")))

        # read and check initial speed is what we expect
        assert(abs( float(self.read_state("docksys.step_angle")) - 0.032 ) < 0.0001) # will take 180/.032 = 5625 cycles to complete
        assert(int(self.read_state("docksys.step_delay")) == 4000)


        self.logger.put("\nTesting docking and undocking with initial step angle of 0.032 and step delay of 4000. \nDocking and Undocking should take 275 cycles.\n")
        self.undock()   #test switch config to undock (rotate 180)
        self.dock()     # tell the system to go to docking config 


        # write different step angle and delay and repeat docking cycle
        # testing data shows step angle is around 180/5000 = 0.036, but 
        # we do not have the theoretical value and it may change with different
        # loading conditions.
        self.logger.put("\nChanging step angle from 0.032 to 0.036 and step delay from 4000 to 2000. \nDocking and Undocking should take 150 cycles.\n")
        self.write_state("docksys.step_angle", "0.036") 
        self.write_state("docksys.step_delay", "2000") 

        self.undock()
        self.dock()

        self.finish()
