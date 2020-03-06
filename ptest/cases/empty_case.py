# Empty test case. Gets cycle count purely for diagnostic purposes
from .base import FlexibleCase

class EmptyCase(FlexibleCase):
    def setup_case_singlesat(self, simulation):
        simulation.flight_controller.write_state("pan.state", 9) # Manual state

    def setup_case_fullmission(self, simulation):
        simulation.flight_controller_follower.write_state("pan.state", 9) # Manual state
        simulation.flight_controller_leader.write_state("pan.state", 9) # Manual state

    def run_case_singlesat(self, simulation):
        simulation.cycle_no = simulation.flight_controller.read_state("pan.cycle_no")

    def run_case_fullmission(self, simulation):
        simulation.cycle_no_follower = simulation.flight_controller_follower.read_state(
                                            "pan.cycle_no")
        simulation.cycle_no_leader = simulation.flight_controller_leader.read_state("pan.cycle_no")

class EmptySimCase(EmptyCase):
    @property
    def run_sim(self):
        return True
