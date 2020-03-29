# SpinMotorsCase. Gets satellite read to spin motors.
from .base import SingleSatOnlyCase

class SpinMotorsCase(SingleSatOnlyCase):
    def setup_case_singlesat(self):
        self.sim.flight_controller.write_state("pan.state", 11) # Mission State = Manual
        self.sim.flight_controller.write_state("adcs.state", 5) # ADCS State = Manual
        self.sim.flight_controller.write_state("adcs_cmd.rwa_mode", 1) # Speed Control
        self.sim.flight_controller.write_state("adcs_cmd.rwa_speed_cmd", "0, 0, 0") # 0 speed to begin with
        self.sim.flight_controller.write_state("dcdc.ADCSMotor_cmd", "true")

    def run_case_singlesat(self):
        self.cycle_no = self.sim.flight_controller.read_state("pan.cycle_no")
        self.finish()
