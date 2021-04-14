"""Day-In-The-Life (DITL) testcase used to check battery health.

The original intention here was to run the satellite actuators using realistic
actuator commands to see how long the satellite can run on battery power to gain
insight into battery health.

However, this test has now been standardized to ensure we can compare between
data points over time.

The test proceeds as follows:

 1. Run the wheels as 15.0,10.0,10.0 for fifteen minutes.
 2. Set the wheel speed to 10.0,10.0,10.0 for the remainder of the case.
 3. The case will exit once the battery voltage drops below 7100mV.
 4. Post notes (explaining timestamps of crashes, start, stop, wheel speed,
    etc.), any complete PTest logs, and the most complete plots of battery
    voltage over time to the Google Drive at:

        System Level/Integration and Testing/Battery Testing/<Date>/<Satellite>/

Warning: When running this case the operator must frequently generate plots of
         battery voltage. PTest DOES crash frequently in long duration cases and
         not generating a plot and saving an image of it every couple minutes
         could mean loss of data.

POC: Kyle Krol
"""

from .base import SingleSatCase
from .utils import Enums

class DitlCase(SingleSatCase):

    def post_boot(self):
        self.ws("dcdc.ADCSMotor_cmd", True); self.cycle()
        self.ws("adcs.state", Enums.adcs_states["manual"]); self.cycle()
        self.ws("adcs_cmd.rwa_speed_cmd", [15.0,10.0,10.0]); self.cycle()
        self.ws("adcs_cmd.rwa_mode", Enums.rwa_modes["RWA_SPEED_CTRL"]); self.cycle()

    def run(self):
        # Run until the battery voltage drops below 7100 mV
        while self.rs("gomspace.vbatt") > 7100:
            self.rs("gomspace.cursys")
            self.rs("adcs_monitor.rwa_speed_rd")
            self.rs("adcs_monitor.mag1_vec")
            self.rs("adcs_monitor.mag2_vec")
            self.rs("pan.cycle_no")
            self.cycle()

        self.finish()
