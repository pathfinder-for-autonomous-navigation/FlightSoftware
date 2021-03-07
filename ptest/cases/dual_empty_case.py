# Empty test case. Gets cycle count purely for diagnostic purposes
from .base import MissionCase
from .utils import Enums, TestCaseFailure
from psim.sims import DualAttitudeOrbitGnc

class DualEmptyCase(MissionCase):
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
        self.finish()

class DualEmptySimCase(DualEmptyCase):
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
    def scrape_uplinks(self):
        return True