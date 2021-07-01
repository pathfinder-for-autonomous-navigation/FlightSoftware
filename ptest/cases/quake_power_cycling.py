# Empty test case. Gets cycle count purely for diagnostic purposes
from .base import SingleSatCase, PSimCase
from psim.sims import SingleAttitudeOrbitGnc
from .utils import Enums, TestCaseFailure


class QuakePowerCycling(SingleSatCase):
    def __init__(self, *args, **kwargs):
        super(QuakePowerCycling, self).__init__(*args, **kwargs)

    #    self.psim_configs += ["truth/standby"]
    #    self.initial_state = "standby"

    def post_boot(self):
        self.mission_state = "follower"
        self.ws("fault_handler.enabled", True)
        self.ws("qfh.enabled", True)
        self.powercycled = True

    def check_powercycle(self):
        return self.rs("gomspace.power_cycle_output3_cmd")

    def radio_state(self):
        return self.rs('radio.state')

    def qfh_state(self):
        return self.rs('qfh.state')

    def is_radio_disabled(self):
        return self.radio_state() == Enums.radio_states["disabled"]

    def check_radio_in_config(self):
        if not self.radio_state() == Enums.radio_states["config"]:
            raise TestCaseFailure("Radio should be in config after QuakeFaultHandler power cycled.")

    def is_powercycle_state(self):
        return self.qfh_state() in [Enums.qfh_states["powercycle_1"], Enums.qfh_states["powercycle_2"], Enums.qfh_states["powercycle_3"]]

    # Cycle until the QFH transitions to the next state
    def advance_to_next_qfh_state(self, time):
        for i in range(time):
            self.cycle()
            self.diagnostics()
            self.cycles_since_blackout_start += 1

            # Check mission state when in QFH forced standby
            if self.qfh_state() == Enums.qfh_states['forced_standby'] and not self.mission_state == "standby":
                raise TestCaseFailure(f"QuakeFaultHandler did not force satellite into standby during during QuakeFaultHandler's `forced standby` state. State was: {self.mission_state}. Current control cycle: {self.rs('pan.cycle_no')}")

            # Check that radio is in config after power cycling
            if self.cycles_since_blackout_start == 1 and self.is_powercycle_state():
                self.check_radio_in_config()

            # Check power cycle behavior
            self.check_correct_powercycle_behavior(time)

    def check_correct_powercycle_behavior(self, time):
        powercycled = self.check_powercycle()
        if powercycled and not self.powercycled:
            self.logger.put("QFH powercycled the radio")

            # If radio is not in disabled, QFH should not power cycle
            if not self.is_radio_disabled() and powercycled:
                raise TestCaseFailure(f"QuakeFaultHandler power cycled the output channel when QuakeManager was not disabled. Cycle: {self.rs('pan.cycle_no')}. Radio state: {self.rs('radio.state')}")

            # Radio should power cycle if:
            #    - QFH is going to enter a power cycle state
            #    - Radio was set to disabled before power cycling
            #    - Cycles since blackout is as much as [time]
            should_powercycle = self.cycles_since_blackout_start >= time and self.is_radio_disabled() and self.is_powercycle_state()

            if (powercycled and not should_powercycle) or (should_powercycle and not powercycled):
                raise TestCaseFailure(f"QuakeFaultHandler failed to power cycle the output channel. Cycle: {self.rs('pan.cycle_no')}. Radio state: {self.rs('radio.state')}")
            self.powercycled = True

        if self.powercycled and not powercycled:
            self.powercycled = False

    def diagnostics(self):
        self.rs("radio.state")
        self.rs("qfh.state")
        self.rs("radio.last_comms_ccno")
        self.rs("gomspace.power_cycle_output3_cmd")
        self.rs("pan.state")

    def run(self):
        # The satellite has been in a blackout since startup. Cycle count starts at 1.
        self.cycles_since_blackout_start = self.rs("pan.cycle_no") - 1
        self.ws("radio.state", Enums.radio_states["wait"])

        # This step is necessary in HOOTL, must be commented out in HITL
        self.ws("gomspace.power_cycle_output3_cmd", True)

        # Simulate one day of no comms
        self.logger.put(f"Creating a comms blackout of 24 hours, starting on control cycle: {self.rs('pan.cycle_no')}")
        for i in range(self.one_day_ccno):
            self.cycle()

        if not self.mission_state == "standby":
            raise TestCaseFailure(f"QuakeFaultHandler did not force satellite into standby after 24 hours of no comms. State was: {self.mission_state}. Current control cycle: {self.rs('pan.cycle_no')}")

        # Reset cycles
        self.cycles_since_blackout_start = 0

        # Simulate second day of no comms
        self.logger.put(f"Creating another comms blackout of 24 hours, starting on control cycle: {self.rs('pan.cycle_no')}")
        self.advance_to_next_qfh_state(self.one_day_ccno)

        # Proceed through all 3 power cycle stages, making sure QFH isn't power cycling in a non-wait state
        self.logger.put(f"Entering power cycle stages, starting on control cycle: {self.rs('pan.cycle_no')}")
        for power_cycle_stage in range(3):

            # Mission state should be in standby at the end of the first two power cycle stages
            if power_cycle_stage == 1 or power_cycle_stage == 2:
                if not self.mission_state == "standby":
                    raise TestCaseFailure(f"QuakeFaultHandler did not force satellite into standby after power cycling. State was: {self.mission_state}. Current control cycle: {self.rs('pan.cycle_no')}")

            # Reset cycles
            self.cycles_since_blackout_start = 0
            self.advance_to_next_qfh_state(self.one_day_ccno // 3)

        # Cycle for one day in safehold
        self.advance_to_next_qfh_state(self.one_day_ccno)

        if not self.mission_state == "safehold":
            raise TestCaseFailure(f"QuakeFaultHandler did not force satellite into safehold after power cycling 3 times. State was: {self.mission_state}. Current control cycle: {self.rs('pan.cycle_no')}")

        self.logger.put("QuakeFaultHandler did not inappropriately power cycle the device.")
        self.logger.put("Testcase finished.")

        self.finish()
