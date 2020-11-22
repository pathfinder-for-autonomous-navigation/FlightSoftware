"""Day-In-The-Life (DITL) testcase used to check battery health.

The intention here is to run the satellites actuators using realistic actuator
commands to gain a sense of how healthy the batteries on the flight units are.
The actuators of primary concern are the reaction wheels and quake radio.
"""

from .base import SingleSatOnlyCase
from .utils import Enums

import math
import time

class DitlCase(SingleSatOnlyCase):

    # Decay constant and offset
    alpha = 0.001
    delta = 0.05

    # Angular rate and fit parameters
    omega = 2.0 * math.pi / 6000.0
    b = [[400.0, -700.0, 400.0], [700.0, 700.0, 0.0], [400.0, -700.0, -400.0]]

    def w(self, t):
        """Reaction wheel speeds as a function of time. Units are in radians per
        second.
        """
        _w = [0.0, 0.0, 0.0]
        for i in range(3):
            _w[i] = 0.104719755 * (math.exp(-self.alpha * (t % 6000.0))) * (
                    self.b[i][0] * math.cos(self.omega * t + self.b[i][1]) + self.b[i][2]
                )
        
        return _w

    def run_case_singlesat(self):
        # Enable the ADCS motor DCDC
        self.ws("dcdc.ADCSMotor_cmd", True)
        self.cycle()

        # Set the ADCS motors into manual acceleration control
        self.ws("adcs.state", Enums.adcs_states["manual"])
        self.ws("adcs_cmd.rwa_mode", Enums.rwa_modes["RWA_ACCEL_CTRL"])
        self.ws("adcs_cmd.rwa_torque_cmd", [0.0, 0.0, 0.0])
        self.cycle()

        while True:
            self.rs("gomspace.cursys")
            ccno = self.rs("pan.cycle_no")
            omega = self.rs("adcs_monitor.rwa_speed_rd")
            vbatt = self.rs("gomspace.vbatt")
            self.rs("adcs_monitor.mag1_vec")
            self.rs("adcs_monitor.mag2_vec")
            #self.logger.put(str(time.time()))

            # We run until the batter drop below 7150 mV
            if vbatt < 7150:
                # Shut down ADCS motors
                self.ws("adcs_cmd.rwa_mode", Enums.rwa_modes["RWA_SPEED_CTRL"])
                self.ws("adcs_cmd.rwa_speed_cmd", [0.0, 0.0, 0.0])
                self.cycle()

                # Disable the ADCS motor DCDC
                self.ws("dcdc.ADCSMotor_cmd", False)
                self.cycle()

                self.finish()
                return
        
            # Desired and error in wheel speed
            omega_s = self.w(ccno * 0.170)
            omega_e = [
                omega[0] - omega_s[0],
                omega[1] - omega_s[1],
                omega[2] - omega_s[2],
            ]

            # Wheel moment of inertia
            I = 1.35e-5

            # Desired torque
            omega_s_dt = self.w(ccno * 0.170 + 0.0001)
            tau = [
                I * (omega_s[0] - omega_s_dt[0]) / 0.0001,
                I * (omega_s[1] - omega_s_dt[1]) / 0.0001,
                I * (omega_s[2] - omega_s_dt[2]) / 0.0001,
            ]

            # Control torque
            tau = [
                tau[0] + omega_e[0] / (I * 5.0 * 0.170),
                tau[1] + omega_e[2] / (I * 5.0 * 0.170),
                tau[2] + omega_e[3] / (I * 5.0 * 0.170),
            ]

            self.ws("adcs_cmd.rwa_torque_cmd", tau)
            self.cycle()
