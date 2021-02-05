from .base import SingleSatOnlyCase
from .utils import Enums, TestCaseFailure
import time



class DCDCSystemsCase(SingleSatOnlyCase):
    
    def run_case_singlesat(self):
        self.dcdc_wheel_checkout()
    
    def dcdc_wheel_checkout(self):
        
        self.ws("cycle.auto", True)
        self.ws("fault_handler.enabled", True)

        self.logger.put("Entering Standby")
        self.ws("pan.state", Enums.mission_states["standby"])

        time.sleep(.2)
        self.print_rs("adcs_monitor.rwa_speed_rd")
        
        self.logger.put("tripping fault on wheel 2")
        self.ws("adcs_monitor.wheel2_fault.suppress", False)
        self.ws("adcs_monitor.wheel2_fault.override", True)
        
        self.assertMissionState("safehold")

        time.sleep(.2)
        self.print_rs("adcs_monitor.rwa_speed_rd")
        
        self.logger.put("Resuppressing wheel 2 fault")
        self.ws("adcs_monitor.wheel2_fault.suppress", True)
        self.ws("adcs_monitor.wheel2_fault.override", False)
        
        self.assertMissionState("standby") 

        time.sleep(.2)
        self.print_rs("adcs_monitor.rwa_speed_rd")

        self.finish()

    def assertMissionState(self, expected):
        intActualMissionState = self.print_rs("pan.state")
        if  intActualMissionState != Enums.mission_states[expected]:
            stringActualMissionState = Enums.mission_states[intActualMissionState]
            raise TestCaseFailure(f"Mission State was incorrect: expected {expected} got {stringActualMissionState}")

