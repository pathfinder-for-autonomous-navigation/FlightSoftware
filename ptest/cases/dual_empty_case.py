# Empty test case. Gets cycle count purely for diagnostic purposes
from .base import DualSatCase
from .base import PSimCase
from .utils import Enums, TestCaseFailure
from psim.sims import DualAttitudeOrbitGnc

class DualEmptyCase(DualSatCase):
    @property
    def initial_state_leader(self):
        return "startup"

    @property
    def initial_state_follower(self):
        return "startup"

    def run(self):
        self.finish()

class DualEmptySimCase(DualSatCase, PSimCase):
    def __init__(self, *args, **kwargs):
        super(DualEmptySimCase, self).__init__(*args, **kwargs)

        self.debug_to_console = True
        self.psim_configs += ["truth/deployment"]

    def run(self):
        self.cycle()