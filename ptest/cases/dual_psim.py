from .base import DualSatCase
import time
from psim.sims import DualAttitudeOrbitGnc
import lin
from .utils import str_to_val, Enums
from ..usb_session import USBSession

class DualPSim(DualSatCase):

    @property
    def sim_configs(self):
        configs = ["truth/ci", "truth/base"]
        configs += ["sensors/base"]
        return configs

    @property
    def sim_model(self):
        return DualAttitudeOrbitGnc

    @property
    def sim_mapping(self):
        return "ci_mapping.json"
        
    @property
    def sim_duration(self):
        return float("inf")

    @property
    def initial_state_leader(self):
        return "startup"
    @property
    def initial_state_follower(self):
        return "startup"

    @property
    def fast_boot_leader(self):
        return True
    @property
    def fast_boot_follower(self):
        return True

    @property
    def debug_to_console(self):
        return True

    def setup_post_bootsetup_leader(self):
        '''
        No reason to do this with leader specifically,
        just need an entry point post sim construction
        '''
        self.sim.mock_sensor_validity = True
        return

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

        sat = "not_a_real_name"
        
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
        
    def run_case_fullmission(self):

        self.log_fc_data(self.flight_controller_leader)
        self.log_fc_data(self.flight_controller_follower)
        self.rs_psim_data()
