# Test case that runs in continuous integration to ensure that PTest isn't broken.
from .base import SingleSatOnlyCase
from .utils import Enums, TestCaseFailure
import os

class CICase(SingleSatOnlyCase):
    def run_case_singlesat(self):
        self.sim.cycle_no = int(self.sim.flight_controller.read_state("pan.cycle_no"))
        if self.sim.cycle_no != 1:
            raise TestCaseFailure(f"Cycle number was incorrect: expected {1} got {self.sim.cycle_no}.")
        self.sim.flight_controller.write_state("cycle.start", "true")
        self.sim.cycle_no = int(self.sim.flight_controller.read_state("pan.cycle_no"))
        if self.sim.cycle_no != 2:
            raise TestCaseFailure(f"Cycle number was incorrect: expected {2} got {self.sim.cycle_no}.")

        self.finish()
