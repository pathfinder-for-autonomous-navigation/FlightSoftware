# Empty test case. Gets cycle count purely for diagnostic purposes
from .base import SingleSatOnlyCase
from psim.sims import SingleAttitudeOrbitGnc
from .utils import Enums, TestCaseFailure


class QFHTest(SingleSatOnlyCase):
    @property
    def sim_duration(self):
        return float("inf")

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
    def sim_duration(self):
        return float("inf")

    @property
    def desired_initial_state(self):
        return "standby"

    @property
    def fast_boot(self):
        return True

    def setup_post_bootsetup(self):
        self.ws("fault_handler.enabled", True)

    def diagnostics(self):
        self.read_state("radio.state")
        self.read_state("qfh.state")
        self.read_state("radio.last_comms_ccno")
        self.read_state("gomspace.power_cycle_output3_cmd")

    def advance_to_next_qfh_state(self,time):
        while True:

            self.cycle()
            self.diagnostics()
            self.cycles_since_blackout_start += 1

            if (self.cycles_since_blackout_start >= time):
                break

    def run_case_singlesat(self):
      # The satellite has been in a blackout since startup. Cycle count starts at 1.
        self.cycles_since_blackout_start = self.rs("pan.cycle_no") - 1
        self.ws("radio.state",0)
        self.ws("gomspace.power_cycle_output3_cmd",False)

        # Simulate one day of no comms
        self.logger.put(f"Creating a comms blackout of 24 hours, starting on control cycle: {self.rs('pan.cycle_no')}")
        while self.cycles_since_blackout_start < self.one_day_ccno:
            self.cycle()
            self.diagnostics()
            self.cycles_since_blackout_start += 1
            
        # Reset cycles
        self.cycles_since_blackout_start = 0

        # Simulate second day of no comms
        self.logger.put(f"Creating another comms blackout of 24 hours, starting on control cycle: {self.rs('pan.cycle_no')}")
        self.advance_to_next_qfh_state(self.one_day_ccno)
       
        # Reset cycles
        self.logger.put(f"Power cycle 1, starting on control cycle: {self.rs('pan.cycle_no')}")
        self.cycles_since_blackout_start = 0
        self.advance_to_next_qfh_state(self.one_day_ccno/3)

        self.logger.put(f"Power cycle 2, starting on control cycle: {self.rs('pan.cycle_no')}")
        self.cycles_since_blackout_start = 0
        self.advance_to_next_qfh_state(self.one_day_ccno/3)

        self.logger.put(f"Power cycle 3, starting on control cycle: {self.rs('pan.cycle_no')}")
        self.cycles_since_blackout_start = 0
        self.advance_to_next_qfh_state(self.one_day_ccno)
        
        self.finish()