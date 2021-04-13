# Empty test case. Gets cycle count purely for diagnostic purposes
from .base import SingleSatCase
from .utils import Enums, TestCaseFailure
from psim.sims import SingleAttitudeOrbitGnc

class EmptyCase(SingleSatCase):
    def run(self):
        self.cycle_no = self.flight_controller.read_state("pan.cycle_no")
        self.finish()

class FailingEmptyCase(SingleSatCase):
    def run_case_singlesat(self):
        raise TestCaseFailure("Deliberate failure intended to test failure in CI.")

    def run_case_fullmission(self):
        raise TestCaseFailure("Deliberate failure intended to test failure in CI.")

class EmptySimCase(EmptyCase):
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

class FailingEmptySimCase(EmptyCase):


    def run_case_singlesat(self):
        raise TestCaseFailure("Deliberate failure intended to test failure in CI.")

class NothingCase(SingleSatCase):
    def setup_case_singlesat(self):
        pass

    def run_case_singlesat(self):
        self.finish()
