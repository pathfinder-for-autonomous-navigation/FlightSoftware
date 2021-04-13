from .base import SingleSatCase
from psim.sims import SingleAttitudeOrbitGnc
from .utils import Enums
from .utils import Enums, TestCaseFailure
from .utils import Enums, mag_of, sum_of_differentials
import time, threading


class SafeholdStandbyTransitionCase(SingleSatCase):

  #Bool fields so output is not spammed with what state spacecraft is in
  faultTriggered = False
  firstStandby = True
  firstSafehold = True
  firstDetumble = True
  firstStartup = True
  tempTime = 0
  
  @property
  def debug_to_console(self):
    return True

  @property
  def sim_duration(self):
    return float("inf")

  @property
  def initial_state(self):
    return "startup"

  def setup_pre_bootsetup(self):
    self.ws("cycle.auto", False)
      
  def setup_post_bootsetup(self):
    self.ws("fault_handler.enabled", True)
    self.logger.put("[TESTCASE] Fault handler enabled")

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
    state = self.rs("pan.state")
    self.dcdc_wheel_checkout(state)

  def dcdc_wheel_checkout(self, currState):
    currCycle = self.rs("pan.cycle_no")
    
    #Case 0: Startup
    if currState == 0 : 
      if self.firstStartup:
        self.logger.put("Starting Up")
        self.firstStartup = False

    #Case 1: Detumble
    elif currState == 1 :
      self.firstStartup = True
      if self.firstDetumble:
        self.logger.put("Detumbling")
        self.firstDetumble = False

    #Case 3: Standby
    elif currState == 3 :
      self.firstDetumble = True
      if(self.firstStandby):
        self.logger.put("In Standby")
        self.tempTime = currCycle
        self.firstStandby = False
      else:
        if currCycle - self.tempTime < 70:
          #do nothing
          pass
        else:
          self.firstStandby = True
          if not self.faultTriggered:
            #trigger the fault 
            self.faultTriggered = True
            self.logger.put("tripping fault on wheel 2")
            self.ws("adcs_monitor.wheel2_fault.suppress", False)
            self.ws("adcs_monitor.wheel2_fault.override", True)

            time.sleep(1)
            self.print_rs("adcs_monitor.wheel2_fault.override")
            self.print_rs("adcs_monitor.wheel2_fault.suppress")
          else:
            #Finish test case (successfully returned to Standby)
            self.logger.put("[TESTCASE] Success")
            self.finish()
          
    #Case 10: Safehold
    elif currState == 10:
      if(self.firstSafehold):
        self.logger.put("In Safehold")
        self.tempTime = currCycle
        self.firstSafehold = False
      else:
        if currCycle - self.tempTime < 20:
          # about 3.4 seconds with sped up FSW (170 ms cc)
          pass
        else:
          self.logger.put("supressing wheel 1 fault")
          self.ws("adcs_monitor.wheel1_fault.suppress", True)
          time.sleep(.2)
          self.print_rs("pan.state")
          self.print_rs("adcs_monitor.rwa_speed_rd")

          self.logger.put("Resuppressing wheel 2 fault")
          self.ws("adcs_monitor.wheel2_fault.suppress", True)
          self.ws("adcs_monitor.wheel2_fault.override", False)
          self.ws("pan.state", 0)