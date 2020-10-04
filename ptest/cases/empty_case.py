# Empty test case. Gets cycle count purely for diagnostic purposes
from .base import SingleSatOnlyCase
from .utils import Enums

class EmptyCase(SingleSatOnlyCase):
    def run_case_singlesat(self):
        self.cycle_no = self.flight_controller.read_state("pan.cycle_no")
        self.finish()

    def run_case_fullmission(self):
        self.cycle_no_follower = self.flight_controller_follower.read_state(
                                            "pan.cycle_no")
        self.cycle_no_leader = self.flight_controller_leader.read_state("pan.cycle_no")
        self.finish()

class EmptySimCase(EmptyCase):
    @property
    def sim_duration(self):
        return float("inf")

class NothingCase(SingleSatOnlyCase):
    def setup_case_singlesat(self):
        pass

    def run_case_singlesat(self):
        self.finish()
