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