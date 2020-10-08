from .base import SingleSatOnlyCase
from .utils import FSWEnum, Enums, BootUtil, TestCaseFailure

class SafeholdReboot(SingleSatOnlyCase):
    @property
    def sim_duration(self):
        return float("inf")

    @property
    def initial_state(self):
        return "standby"

    @property
    def fast_boot(self):
        return False

    def run_case_singlesat(self):
        if not hasattr(self, "test_stage"):
            self.test_stage = "force_fault"

        if self.test_stage == "force_fault":
            self.logger.put("Overriding Gomspace low-battery fault.")
            self.ws("gomspace.low_batt.suppress", False)
            self.ws("gomspace.low_batt.override", True)
            self.test_stage = "safehold"

        elif self.test_stage == "safehold":
            if self.mission_state != "safehold":
                raise TestCaseFailure("Satellite did not go to safehold after Gomspace low-battery fault was forced.")
            else:
                self.logger.put("Satellite now in in safehold state.")
                self.test_stage = "reboot"

        elif self.test_stage == "reboot":
            self.logger.put("Expecting satellite to reboot.")
            self.test_stage = "await_reboot"

        elif self.test_stage == "await_reboot":
            self.finish()
