from .base import SingleSatCase, PSimCase
from .utils import FSWEnum, Enums, TestCaseFailure
import lin 

class SafeholdReboot(SingleSatCase, PSimCase):
    def __init__(self, *args, **kwargs):
        super(SafeholdReboot, self).__init__(*args, **kwargs)
        self.initial_state = "standby"
        self.psim_configs += ['truth/standby']

    def post_boot(self):
        self.ws('fault_handler.enabled', True)

    def run(self):
        self.cycle()
        
        if not hasattr(self, "test_stage"):
            self.test_stage = "force_fault"

        if self.test_stage == "force_fault":
            self.logger.put("Overriding Gomspace low-battery fault.")
            self.ws("gomspace.low_batt.suppress", False)
            self.ws("gomspace.low_batt.override", True)
            self.test_stage = "safehold"

        elif self.test_stage == "safehold":
            print(self.mission_state)
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
