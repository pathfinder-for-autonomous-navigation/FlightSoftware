# Test case that runs in continuous integration to ensure that PTest isn't broken.
from .base import SingleSatOnlyCase, TestCaseFailure
import os

class CICase(SingleSatOnlyCase):
    @property
    def sim_duration(self):
        # Sim cannot run in CI since there is no MATLAB installation, so this value must be zero.
        return 0

    def setup_case_singlesat(self):
        self.sim.flight_controller.write_state("pan.state", self.mission_states.get_by_name("manual"))

    def setup_case_fullmission(self):
        self.sim.flight_controller_follower.write_state("pan.state", self.mission_states.get_by_name("manual"))
        self.sim.flight_controller_leader.write_state("pan.state", self.mission_states.get_by_name("manual"))

    def run_case_singlesat(self):
        self.sim.cycle_no = int(self.sim.flight_controller.read_state("pan.cycle_no"))
        if self.sim.cycle_no != 1:
            raise TestCaseFailure(f"Cycle number was incorrect: expected {1} got {self.sim.cycle_no}.")
        self.sim.flight_controller.write_state("cycle.start", "true")
        self.sim.cycle_no = int(self.sim.flight_controller.read_state("pan.cycle_no"))
        if self.sim.cycle_no != 2:
            raise TestCaseFailure(f"Cycle number was incorrect: expected {2} got {self.sim.cycle_no}.")
        
        self.finish()
