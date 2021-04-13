from .base import DualSatCase
from .psim_case import PSimCase
import time
from psim.sims import DualAttitudeOrbitGnc
import lin
from .utils import str_to_val, Enums
from ..usb_session import USBSession

class DualPSim(DualSatCase, PSimCase):

    def __init__(self, *args, **kwargs):
        super(DualPSim, self).__init__(*args, **kwargs)

        self.debug_to_console = True
        self.psim_configs += ["truth/deployment"]

    def log_fc_data(self, usb_device: USBSession):
        
        fc_states = ["pan.deployment.elapsed",
        "pan.state",
        "radio.state",
        "piksi.state",
        "pan.cycle_no",
        "pan.bootcount",
        "adcs.state",
        "piksi.time",
        "orbit.pos",
        "orbit.vel",
        "adcs_monitor.ssa_vec",
        "adcs_monitor.mag1_vec",
        "adcs_monitor.gyr_vec",

        'adcs_monitor.ssa_mode',

        "attitude_estimator.q_body_eci",
        "attitude_estimator.L_body",
        # "attitude_estimator.fro_P",
        "adcs_cmd.mtr_cmd",
        "adcs_cmd.rwa_torque_cmd",
        
        "attitude_estimator.valid",
        "attitude_estimator.w_bias_body",
        "orbit.valid"
        ]

        for fc_state in fc_states:
            usb_device.smart_read(fc_state)
        
    def rs_psim_data(self):
        
        psim_states = [
        "truth.t.ns",
        "truth.dt.ns"]
        
        psim_per_sat_states = [
        'truth.{sat}.attitude.w',
        'truth.{sat}.attitude.L',
        'truth.{sat}.wheels.t',
        'truth.{sat}.wheels.w',
        'truth.{sat}.orbit.r.ecef',
        'truth.{sat}.orbit.v.ecef',
        ]
        
        sats = ['follower', 'leader']

        for sat_name in sats:
            for psim_fn in psim_per_sat_states:
                psim_full_name = psim_fn.format(sat = sat_name)
                self.rs_psim(psim_full_name)

        for psim_state in psim_states:
            self.rs_psim(psim_state)
        
    def run(self):
        self.cycle()

        self.log_fc_data(self.flight_controller_leader)
        self.log_fc_data(self.flight_controller_follower)
        self.rs_psim_data()
