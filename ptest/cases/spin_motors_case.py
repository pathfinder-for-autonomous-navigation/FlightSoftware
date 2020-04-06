# SpinMotorsCase. Gets satellite read to spin motors.
from .base import SingleSatOnlyCase
import time

class SpinMotorsCase(SingleSatOnlyCase):
    def setup_case_singlesat(self):
        self.ws("pan.state", self.mission_states.get_by_name("manual"))
        self.ws("dcdc.ADCSMotor_cmd", True)
        self.cycle()

        self.ws("adcs.state", self.adcs_states.get_by_name("point_manual"))
        self.ws("adcs_cmd.rwa_mode", self.rwa_modes.get_by_name("RWA_SPEED_CTRL"))
        self.ws("adcs_cmd.rwa_speed_cmd", [50,50,50])
        self.cycle()

    def run_case_singlesat(self):
        self.cycle_no = self.sim.flight_controller.read_state("pan.cycle_no")
        self.finish()
