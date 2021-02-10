from .base import SingleSatOnlyCase
from psim.sims import SingleAttitudeOrbitGnc
from .utils import Enums
from .utils import Enums, TestCaseFailure
from .utils import Enums, mag_of, sum_of_differentials
import time, threading



class DCDCWheelCase(SingleSatOnlyCase):
    faultTriggered = False
    @property
    def sim_configs(self):
        configs = ["truth/ci", "truth/base"]
        configs += ["sensors/base"]
        return configs

    @property
    def sim_model(self):
        return SingleAttitudeOrbitGnc

    @property
    def sim_mapping(self):
        return "ci_mapping.json"

    @property
    def debug_to_console(self):
        return True

    @property
    def sim_duration(self):
        return float("inf")

    @property
    def initial_state(self):
        return "startup"

    @property
    def fast_boot(self):
        return True

    @property
    def sim_initial_state(self):
        return "startup"

    @property
    def sim_ic_map(self):
        ret = {}
        ret["truth.t.ns"] = 420000000*10
        #ret["truth.leader.attitude.w"] = [1,2,3]
        return ret
    def data_logs(self):

        self.rs("pan.deployment.elapsed")
        self.rs("pan.state")
        self.rs("pan.cycle_no")
        self.rs("pan.bootcount")
        self.rs("adcs.state")
        self.rs("attitude_estimator.q_body_eci")
        self.rs("attitude_estimator.w_body")
        self.rs("attitude_estimator.fro_P")
        self.rs("adcs_cmd.rwa_torque_cmd")
    
    def run_case_singlesat(self):
      self.rs_psim("truth.t.ns")
      self.rs_psim("truth.dt.ns")
      self.rs_psim("truth.leader.attitude.w")
      self.data_logs()
      self.dcdc_wheel_checkout(self.rs("pan.state"))
      

    def dcdc_wheel_checkout(self, currState):
        self.ws("fault_handler.enabled", True)
        
        if currState == 0 :
          self.logger.put("Starting up")
        elif currState == 1 :
          self.logger.put("Detumbling")
        elif currState == 3 :
          self.logger.put("In Standby")
          if not self.faultTriggered:
            #trigger the fault 
            self.faultTriggered = True
            self.logger.put("tripping fault on wheel 2")
            self.ws("adcs_monitor.wheel2_fault.suppress", False)
            self.ws("adcs_monitor.wheel2_fault.override", True)
            time.sleep(0.2)
            self.print_rs("adcs_monitor.wheel2_fault.override")
            self.print_rs("adcs_monitor.wheel2_fault.suppress")
          else:
            #Finish test case (successfully returned to Standby)
            self.finish()
        elif currState == 10:
          self.logger.put("In Safehold")

          self.logger.put("supressing wheel 1 fault")
          self.ws("adcs_monitor.wheel1_fault.suppress", True)
          time.sleep(.2)
          self.print_rs("pan.state")
          self.print_rs("adcs_monitor.rwa_speed_rd")

          self.logger.put("Resuppressing wheel 2 fault")
          self.ws("adcs_monitor.wheel2_fault.suppress", True)
          self.ws("adcs_monitor.wheel2_fault.override", False)
          self.ws("pan.state", 0)