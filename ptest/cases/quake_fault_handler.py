from .base import SingleSatOnlyCase
from .utils import FSWEnum, Enums, TestCaseFailure, BootUtil

class QuakeFaultHandler_Fast(SingleSatOnlyCase):
    @property
    def sim_duration(self):
        return float("inf")

    @property
    def initial_state(self):
        return "leader"

    @property
    def fast_boot(self):
        return False

    @property
    def one_day_ccno(self):
        return 60 * 1000 // 170

    @property
    def compilation_notice(self):
        return \
        """
        NOTE: This test requires Flight Software to be compiled with the
        \"SPEEDUP\" flag so that the timeouts for the Quake Fault
        Handler only take minutes, not hours.\n
        """

    def setup_pre_bootsetup(self):
        self.logger.put(self.compilation_notice)

    def collect_diagnostic_data(self):
        self.rs("qfh.state")
        self.rs("pan.state")
        self.rs("gomspace.power_cycle_output1_cmd")
        self.rs("pan.cycle_no")

    def run_case_singlesat(self):
        self.collect_diagnostic_data()

        if not hasattr(self, "test_stage"):
            self.test_stage = "first_no_comms"
            self.logger.put(f"Creating a comms blackout of 24 hours, starting on control cycle {self.rs('pan.cycle_no')}")

        if self.test_stage == "first_no_comms":
            self.comms_blackout_timer += 1
            if self.comms_blackout_timer > self.one_day_ccno:
                if not self.mission_state == "standby":
                    raise TestCaseFailure(f"QuakeFaultHandler did not force satellite into standby after 24 hours of no comms. State was: {self.mission_state}")
                else:
                    self.test_stage = "powercycles"
                    self.comms_blackout_timer = 0
                    self.logger.put("Comms blackout forced a transition into the standby state.")
                    self.logger.put(f"Creating a comms blackout of 8 additional hours, starting on control cycle {self.rs('pan.cycle_no')}")

        elif self.test_stage == "powercycles":
            if not hasattr(self, "powercycles_count"):
                self.powercycles_count = 0

            self.comms_blackout_timer += 1

            if not self.mission_state == "standby":
                raise TestCaseFailure(f"State of spacecraft was not `standby` during Quake Fault Handler's powercycling states. State was: {self.mission_state}")

            if self.comms_blackout_timer > self.one_day_ccno // 3:
                # TODO check if Quake radio is off
                self.powercycles_count += 1
                self.comms_blackout_timer = 0
                self.logger.put("Comms blackout caused a powercycle of Quake.")
                self.logger.put(f"Creating a comms blackout of 8 additional hours, starting on control cycle {self.rs('pan.cycle_no')}")
                if self.powercycles_count > 3:
                    self.test_stage = "safehold"

        elif self.test_stage == "safehold":
            self.comms_blackout_timer += 1
            if self.comms_blackout_timer > self.one_day_ccno // 3:
                if not self.mission_state == "safehold":
                    raise TestCaseFailure(f"QuakeFaultHandler did not force satellite into safehold after 48 hours of no comms. State was: {self.mission_state}")
                else:
                    self.logger.put(f"The 48-hour total comms blackout caused a mission state transition to safehold on control cycle {self.rs('pan.cycle_no')}")
                    self.logger.put("Testcase finished.")
                    self.finish()

class QuakeFaultHandler_Realtime(QuakeFaultHandler_Fast):
    @property
    def compilation_notice(self):
        return \
        """
        NOTE: This test requires Flight Software to be compiled without the
        \"SPEEDUP\" flag so that the timeouts for the Quake Fault
        Handler are realistic.\n
        """

    @property
    def one_day_ccno(self):
        return 24 * 60 * 1000 // 170
