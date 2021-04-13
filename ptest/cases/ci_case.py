# Test case that runs in continuous integration to ensure that PTest isn't broken.
from .base import SingleSatCase
from .utils import Enums, TestCaseFailure
import os

class CICase(SingleSatCase):
    def run(self):
        self.cycle_no = int(self.flight_controller.read_state("pan.cycle_no"))
        if self.cycle_no != 1:
            raise TestCaseFailure(f"Cycle number was incorrect: expected {1} got {self.cycle_no}.")
        self.flight_controller.write_state("cycle.start", "true")
        self.cycle_no = int(self.flight_controller.read_state("pan.cycle_no"))
        if self.cycle_no != 2:
            raise TestCaseFailure(f"Cycle number was incorrect: expected {2} got {self.cycle_no}.")

        self.finish()
