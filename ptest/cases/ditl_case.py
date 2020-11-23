"""Day-In-The-Life (DITL) testcase used to check battery health.

The intention here is to run the satellites actuators using realistic actuator
commands to gain a sense of how healthy the batteries on the flight units are.
The actuators of primary concern are the reaction wheels and quake radio.
"""

from .base import SingleSatOnlyCase
from .utils import Enums

import time

class SpinMotorsCase(SingleSatOnlyCase):
    def setup_post_bootsetup(self):
        self.ws("dcdc.ADCSMotor_cmd", True); self.cycle()
        self.ws("adcs.state", Enums.adcs_states["manual"]); self.cycle()
        self.ws("adcs_cmd.rwa_speed_cmd", [10.0,10.0,10.0]); self.cycle()
        self.ws("adcs_cmd.rwa_mode", Enums.rwa_modes["RWA_SPEED_CTRL"]); self.cycle()

    def run_case_singlesat(self):
        # Run until the battery voltage drops below 7000 mV
        while self.rs("gomspace.vbatt") > 7000:
            self.rs("gomspace.cursys")
            self.rs("adcs_monitor.rwa_speed_rd")
            self.rs("adcs_monitor.mag1_vec")
            self.rs("adcs_monitor.mag2_vec")
            self.rs("pan.cycle_no")
            self.cycle()

        self.finish()
