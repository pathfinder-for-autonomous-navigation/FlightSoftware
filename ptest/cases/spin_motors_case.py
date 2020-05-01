# SpinMotorsCase. Spins the satellite wheels. Verified working - 04/06 for PR #335
from .base import SingleSatOnlyCase
import time

class SpinMotorsCase(SingleSatOnlyCase):
    def setup_case_singlesat(self):
        self.ws("pan.state", self.mission_states["manual"])
        self.ws("dcdc.ADCSMotor_cmd", True)
        self.cycle()

        self.ws("adcs.state", self.adcs_states["point_manual"])
        self.ws("adcs_cmd.rwa_mode", self.rwa_modes["RWA_SPEED_CTRL"])
        self.ws("adcs_cmd.rwa_speed_cmd", [10,10,10])
        self.cycle()

    def run_case_singlesat(self):
        self.cycle_no = self.sim.flight_controller.read_state("pan.cycle_no")
        self.finish()
