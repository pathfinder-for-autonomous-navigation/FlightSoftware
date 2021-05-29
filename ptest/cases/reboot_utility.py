# Case that reboots the satellite (by setting the reset flag). Utility purposes only.
from .base import SingleSatCase
from .utils import Enums, TestCaseFailure
import os

class Reboot(SingleSatCase):

    def pre_boot(self):
        self.ws("pan.state", Enums.mission_states["manual"])
        self.initial_state = "manual"
        
    def run(self):
        self.mission_state = "manual"
        self.cycle()

        self.ws("gomspace.gs_reset_cmd", True)
        self.cycle()
        self.finish()
