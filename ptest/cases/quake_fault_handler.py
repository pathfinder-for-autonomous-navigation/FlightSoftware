# Runs mission from startup state to standby state.
from .base import SingleSatOnlyCase, TestCaseFailure
from .mission import MissionStateMachineUtilities

class QuakeFaultHandler_Fast(SingleSatOnlyCase, MissionStateMachineUtilities):
    @property
    def sim_duration(self):
        return float("inf")

    @property
    def sim_initial_state(self):
        # Start the spacecraft with zero angular rate in the body frame, 
        # so that we exit detumble almost immediately.
        return "startup"

    @property
    def mission_state(self):
        return self.mission_states[int(self.sim.flight_controller.read_state("pan.state"))]

    @property
    def qfh_state(self):
        return self.sim.flight_controller.read_state("qfh.state")

    def setup_case_singlesat(self):
        self.setup_initial_conditions()
        self.setup_boot_to_standby()

        self.one_day_ccno = 60 * 1000 / 170
        self.comms_blackout_timer = 0
        self.logger.put("""
NOTE: This test requires Flight Software to be compiled with the
\"SHORTEN_DAYS\" flag so that the timeouts for the Quake Fault
Handler only take minutes, not hours.\n""")
        self.logger.put("[TESTCASE] Waiting for the satellite to boot to the leader state.")

        self.ws("fault_handler.enabled", True)

        self.test_stage = "waiting_for_boot"

    def run_case_singlesat(self):
        # Collect quake fault handler state on every control cycle so that
        # we have a record of the data.
        _ = self.qfh_state

        if self.test_stage == "waiting_for_boot" and self.run_boot_to_leader():
            self.logger.put("[TESTCASE] Satellite has booted to leader. Starting a 24-hours comms blackout (in satellite time; real duration of blackout is 1 min).")
            self.test_stage = "first_no_comms" 

        elif self.test_stage == "first_no_comms":
            if self.comms_blackout_timer > self.one_day_ccno:
                if not self.mission_state == "standby":
                    raise TestCaseFailure("QuakeFaultHandler did not force satellite into standby after 24 hours of no comms.")
                else:
                    self.logger.put("[TESTCASE] Transitioning to the powercycling states of the Quake fault handler.")
                    self.test_stage = "powercycles"
                    self.comms_blackout_timer = 0

        elif self.test_stage == "powercycles":
            if not hasattr(self, "powercycles_count"):
                self.powercycles_count = 0

            if not self.mission_state == "standby":
                raise TestCaseFailure("QuakeFaultHandler did not force satellite into standby after 24 hours of no comms.")

            if self.comms_blackout_timer > self.one_day_ccno / 3:
                # TODO check if Quake radio has been powercycled.

                self.powercycles_count += 1
                self.logger.put(f"[TESTCASE] Powercycled {self.powercycles_count} time(s).")
                self.comms_blackout_timer = 0
                if self.powercycles_count == 3:
                    self.logger.put("[TESTCASE] Transitioning to safehold.")
                    self.test_stage = "safehold"

        elif self.test_stage == "safehold":
            if self.comms_blackout_timer > self.one_day_ccno / 3:
                if not self.mission_state == "safehold":
                    raise TestCaseFailure("QuakeFaultHandler did not force satellite into safehold after 48 hours of no comms.")
                else:
                    self.logger.put("[TESTCASE] Testcase finished.")
                    self.finish()

        self.comms_blackout_timer += 1

class QuakeFaultHandler_Real(QuakeFaultHandler_Fast):
    """
    This testcase is the same as the Fast testcase, just with realistic timeouts for comms.
    """

    @property
    def sim_duration(self):
        return float("inf")

    @property
    def sim_initial_state(self):
        # Start the spacecraft with zero angular rate in the body frame, 
        # so that we exit detumble almost immediately.
        return "startup"

    def setup_case_singlesat(self):
        self.setup_initial_conditions()
        self.setup_boot_to_standby()

        self.one_day_ccno = 24 * 60 * 60 * 1000 / 170
        self.logger.put("""[TESTCASE] 
NOTE: This test requires Flight Software to be compiled with the
\"SHORTEN_DAYS\" flag so that the timeouts for the Quake Fault
Handler only take minutes, not hours.""")
        self.logger.put("[TESTCASE] Waiting for the satellite to boot to standby.")

