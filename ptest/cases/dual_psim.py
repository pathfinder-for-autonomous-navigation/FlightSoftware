from .base import MissionCase
import time
from psim.sims import DualAttitudeOrbitGnc
import lin
from .utils import str_to_val, Enums
from ..usb_session import USBSession

class DualPsim(MissionCase):

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
        return "leader"
    @property
    def initial_state_follower(self):
        return "follower"

    @property
    def fast_boot_leader(self):
        return True
    @property
    def fast_boot_follower(self):
        return True

    def log_fc_data(self, usb_device: USBSession):
        
        fc_states = ["pan.deployment.elapsed",
        "pan.state",
        "pan.cycle_no",
        "pan.bootcount",
        "adcs.state",
        "orbit.time",
        "orbit.pos",
        "orbit.vel",
        "adcs_monitor.ssa_vec",
        "adcs_monitor.mag1_vec",
        "adcs_monitor.gyr_vec",
        "attitude_estimator.q_body_eci",
        "attitude_estimator.w_body",
        "attitude_estimator.fro_P",
        "adcs_cmd.mtr_cmd",
        "adcs_cmd.rwa_torque_cmd"]

        for fc_state in fc_states:
            usb_device.smart_read(fc_state)
        
    def rs_psim_data(self):
        
        psim_states = [
        "truth.leader.attitude.w",
        "truth.t.ns",
        "truth.dt.ns"]
        
        for psim_state in psim_states:
            self.rs_psim(psim_state)
        
    def run_case_fullmission(self):

        self.log_fc_data(self.flight_controller_leader)
        self.log_fc_data(self.flight_controller_follower)
        self.rs_psim_data()
