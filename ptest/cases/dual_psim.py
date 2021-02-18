from .base import MissionCase
import time
from psim.sims import DualAttitudeOrbitGnc
import lin
from .utils import str_to_val, Enums

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

    def run_case_fullmission(self):

        self.flight_controller_leader.read_state("pan.cycle_no")
        self.flight_controller_follower.read_state("pan.cycle_no")
