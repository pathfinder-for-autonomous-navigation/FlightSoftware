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
        """
        This testcase depends on the version of flight software compiled with
        -D SPEEDUP.
        """
        return 10 * 1000 // 170

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

        self.qfh_state = None
        self.powercycle_happening = None

    def setup_post_bootsetup(self):
        self.ws("fault_handler.enabled", True)

    def check_quake_powercycled(self):
        if not self.powercycle_happening:
            self.logger.put("Quake radio was not powercycled.")
        else:
            self.logger.put("Comms blackout caused a powercycle of Quake.")

    def collect_diagnostic_data(self):
        self.qfh_state = self.rs("qfh.state")
        self.rs("pan.state")
        self.powercycle_happening = self.rs("gomspace.power_cycle_output1_cmd")
        self.rs("pan.cycle_no")

    def run_case_singlesat(self):
        if not self.finished:
            self.collect_diagnostic_data()

        if not hasattr(self, "test_stage"):
            # The satellite has been in a blackout since startup.
            #
            # Note: we subtract 1 because the control cycle count starts at 1.
            # So if the current ccno is 1, that implies there have been zero
            # cycles since the blackout started.
            self.cycles_since_blackout_start = self.rs("pan.cycle_no") - 1

            self.test_stage = "first_no_comms"
            self.logger.put(f"Creating a comms blackout of 24 hours (measured from control cycle #1.) Current control cycle: {self.rs('pan.cycle_no')}")

        if self.test_stage == "first_no_comms":
            if self.cycles_since_blackout_start > self.one_day_ccno:
                if not self.mission_state == "standby":
                    raise TestCaseFailure(f"QuakeFaultHandler did not force satellite into standby after 24 hours of no comms. State was: {self.mission_state}. Current control cycle: {self.rs('pan.cycle_no')}")
                else:
                    self.cycles_since_blackout_start = 0
                    self.logger.put("QuakeFaultHandler forced spacecraft into the standby state.")
                    self.logger.put(f"Creating another comms blackout of 24 hours, starting on control cycle {self.rs('pan.cycle_no')}")
                    self.test_stage = "second_no_comms"

        elif self.test_stage == "second_no_comms":
            if not self.mission_state == "standby":
                raise TestCaseFailure(f"State of spacecraft was not `standby` during QuakeFaultHandler's `forced standby` state. State was: {self.mission_state}.  Current control cycle: {self.rs('pan.cycle_no')}")
            if self.cycles_since_blackout_start > self.one_day_ccno:
                self.test_stage = "powercycles"

        elif self.test_stage == "powercycles":
            if not hasattr(self, "powercycles_count"):
                self.logger.put("Comms blackout forced a transition into Quake Fault Handler's powercycling states.")
                self.check_quake_powercycled()
                self.powercycles_count = 0
                self.cycles_since_blackout_start = 0
                self.logger.put(f"Creating a comms blackout of 8 additional hours, starting on control cycle {self.rs('pan.cycle_no')}")

            if self.cycles_since_blackout_start > self.one_day_ccno // 3:
                self.check_quake_powercycled()
                self.powercycles_count += 1
                self.cycles_since_blackout_start = 0
                if self.powercycles_count == 3:
                    self.test_stage = "safehold"
                elif not self.mission_state == "standby":
                    raise TestCaseFailure(f"State of spacecraft was not `standby` during Quake Fault Handler's powercycling states. State was: {self.mission_state}.  Current control cycle: {self.rs('pan.cycle_no')}")
                else:
                    self.logger.put(f"Creating a comms blackout of 8 additional hours, starting on control cycle {self.rs('pan.cycle_no')}")

        elif self.test_stage == "safehold":
            if not self.mission_state == "safehold":
                raise TestCaseFailure(f"QuakeFaultHandler did not force satellite into safehold after 48 hours of no comms. State was: {self.mission_state}.  Current control cycle: {self.rs('pan.cycle_no')}")
            else:
                self.logger.put(f"The 48-hour total comms blackout caused a mission state transition to safehold on control cycle {self.rs('pan.cycle_no')}")
                self.logger.put("Testcase finished.")
                self.test_stage = "finished"
                self.finish()

        self.cycles_since_blackout_start += 1

class QuakeFaultHandler_Realtime(QuakeFaultHandler_Fast):
    @property
    def compilation_notice(self):
        return \
        """
        NOTE: This test requires Flight Software to be compiled without the
        \"SPEEDUP\" flag so that the timeouts for the Quake Fault
        Handler are realistic.\n
        """
