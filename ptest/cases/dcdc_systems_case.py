from .base import SingleSatOnlyCase
from .utils import Enums
import time



class DCDCSystemsCase(SingleSatOnlyCase):

    def run_case_singlesat(self):
        self.dcdc_wheel_checkout()
    
    def dcdc_wheel_checkout(self):
        self.ws("cycle.auto", True)
        self.logger.put("Entering Standby")
        self.ws("pan.state", Enums.mission_states["standby"])

        time.sleep(.2)
        self.print_rs("adcs_monitor.rwa_speed_rd")
        self.ws("adcs_monitor.wheel2_fault.suppress", False)
        self.ws("adcs_monitor.wheel2_fault.override", True)
        self.print_rs("adcs_monitor.wheel2_fault.override")
        self.print_rs("adcs_monitor.wheel2_fault.suppress")
        
        time.sleep(.2)
        self.print_rs("pan.state")
        self.print_rs("adcs_monitor.rwa_speed_rd")
        self.logger.put("supressing wheel 1 fault")
        self.ws("adcs_monitor.wheel1_fault.suppress", True)
        
        self.print_rs("adcs_monitor.rwa_speed_rd")

        self.finish()



