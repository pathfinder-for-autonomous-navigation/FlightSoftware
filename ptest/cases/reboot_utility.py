# Case that reboots the satellite (by setting the reset flag). Utility purposes only.
from .base import SingleSatCase
from .utils import Enums, TestCaseFailure
import os

class Reboot(SingleSatCase):
    def run(self):
        self.ws("gomspace.gs_reset_cmd", True)
        self.cycle()
        self.finish()
