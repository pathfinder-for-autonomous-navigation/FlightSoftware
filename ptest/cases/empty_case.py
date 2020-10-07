# Empty test case. Gets cycle count purely for diagnostic purposes
from .base import SingleSatOnlyCase
from .utils import Enums

class EmptyCase(SingleSatOnlyCase):
    def run_case_singlesat(self):
        self.cycle_no = self.flight_controller.read_state("pan.cycle_no")
        self.finish()

class FailingEmptyCase(SingleSatOnlyCase):
    def run_case_singlesat(self):
        raise self.TestCaseFailure("Deliberate failure intended to test failure in CI.")

    def run_case_fullmission(self):
        raise self.TestCaseFailure("Deliberate failure intended to test failure in CI.")

class EmptySimCase(EmptyCase):
    @property
    def sim_duration(self):
        return float("inf")

class FailingEmptySimCase(EmptyCase):
    @property
    def sim_duration(self):
        return float("inf")

    def run_case_singlesat(self):
        raise self.TestCaseFailure("Deliberate failure intended to test failure in CI.")

class NothingCase(SingleSatOnlyCase):
    def setup_case_singlesat(self):
        pass

    def run_case_singlesat(self):
        self.finish()
