# Case that reboots the satellite. Utility purposes only.
from .base import SingleSatOnlyCase
from .utils import Enums, TestCaseFailure
import os

class Reboot(SingleSatOnlyCase):
    def run_case_singlesat(self):
        self.ws("gomspace.gs_reboot_cmd", True)
        self.cycle
        self.finish()
