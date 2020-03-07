# SpinMotorsCase. Gets satellite read to spin motors.
from .base import SingleSatOnlyCase

class SpinMotorsCase(SingleSatOnlyCase):
    def setup_case_singlesat(self, simulation):
        simulation.flight_controller.write_state("pan.state", 11) # Mission State = Manual
        simulation.flight_controller.write_state("adcs.state", 5) # ADCS State = Manual
        simulation.flight_controller.write_state("adcs_cmd.rwa_mode", 1) # Speed Control
        simulation.flight_controller.write_state("adcs_cmd.rwa_speed_cmd", "0, 0, 0") # 0 speed to begin with
        simulation.flight_controller.write_state("dcdc.ADCSMotor_cmd", "true")

    def setup_case_fullmission(self, simulation):
        simulation.flight_controller_follower.write_state("pan.state", 11) # Manual state
        simulation.flight_controller_leader.write_state("pan.state", 11) # Manual state

    def run_case_singlesat(self, simulation):
        simulation.cycle_no = simulation.flight_controller.read_state("pan.cycle_no")

    def run_case_fullmission(self, simulation):
        simulation.cycle_no_follower = simulation.flight_controller_follower.read_state(
                                            "pan.cycle_no")
        simulation.cycle_no_leader = simulation.flight_controller_leader.read_state("pan.cycle_no")
