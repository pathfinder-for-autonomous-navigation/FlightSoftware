# SpinMotorsCase. Spins the satellite wheels. Verified working - 04/06 for PR #335
from .base import SingleSatCase
from .utils import Enums
import time

class SpinMotorsCase(SingleSatCase):
    def post_boot(self):
        self.ws("dcdc.ADCSMotor_cmd", True)
        self.cycle()

        self.ws("adcs.state", Enums.adcs_states["manual"])
        self.ws("adcs_cmd.rwa_mode", Enums.rwa_modes["RWA_SPEED_CTRL"])
        self.ws("adcs_cmd.rwa_speed_cmd", [10,10,10])
        self.cycle()

    def run_case_singlesat(self):
        self.cycle_no = self.flight_controller.read_state("pan.cycle_no")
        self.finish()
