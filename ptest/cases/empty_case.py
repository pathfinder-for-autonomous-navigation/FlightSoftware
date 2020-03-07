# Empty test case. Gets cycle count purely for diagnostic purposes
from .base import FlexibleCase

class EmptyCase(FlexibleCase):
    def setup_case_singlesat(self):
        self.sim.flight_controller.write_state("pan.state", self.mission_states["manual"])

    def setup_case_fullmission(self):
        self.sim.flight_controller_follower.write_state("pan.state", self.mission_states["manual"])
        self.sim.flight_controller_leader.write_state("pan.state", self.mission_states["manual"])

    def run_case_singlesat(self):
        self.sim.cycle_no = self.sim.flight_controller.read_state("pan.cycle_no")

    def run_case_fullmission(self):
        self.sim.cycle_no_follower = self.sim.flight_controller_follower.read_state(
                                            "pan.cycle_no")
        self.sim.cycle_no_leader = self.sim.flight_controller_leader.read_state("pan.cycle_no")

class EmptySimCase(EmptyCase):
    @property
    def sim_duration(self):
        return float("inf")
