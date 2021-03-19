# Empty test case. Gets cycle count purely for diagnostic purposes
from .base import SingleSatOnlyCase
from psim.sims import SingleAttitudeOrbitGnc
from .utils import Enums, TestCaseFailure


class QuakePowerCycling(SingleSatOnlyCase):
    @property
    def desired_initial_state(self):
        return "standby"

    @property
    def fast_boot(self):
        return True

    def setup_post_bootsetup(self):
        self.ws("fault_handler.enabled", True)

    def check_powercycle(self):
        return self.rs("gomspace.power_cycle_output3_cmd")

    def is_radio_disabled(self):
        return self.rs("radio.state") == Enums.radio_states["disabled"] 

    def check_radio_in_config(self):
        if not self.rs("radio.state") == Enums.radio_states["config"]:
            raise TestCaseFailure("Radio should be in config after QuakeFaultHandler power cycled.")

    def is_powercycle_state(self):
        return self.rs("qfh.state") in [Enums.qfh_states["powercycle_1"],Enums.qfh_states["powercycle_2"] ,Enums.qfh_states["powercycle_3"]]

    # Cycle until the QFH transitions to the next state
    def advance_to_next_qfh_state(self,time):
        for i in range(time):
            self.cycle()
            self.diagnostics()
            self.cycles_since_blackout_start += 1
            
            # Check that radio is in config after power cycling
            if self.cycles_since_blackout_start == 1 and self.is_powercycle_state():
                self.check_radio_in_config()

            # Check power cycle behavior
            self.check_correct_powercycle_behavior(time)

    def check_correct_powercycle_behavior(self,time):
        powercycled = self.check_powercycle()
        output = self.rs("gomspace.output.output3")
        
        # Unsure how this will translate in HITL with cycles and timing
        # if powercycled and not output:
        #     raise TestCaseFailure("QuakeFaultHandler power cycled the output channel when powercycle flag was set.")
        
        # If radio is not in disabled, QFH should not power cycle
        if not self.is_radio_disabled() and powercycled:
            raise TestCaseFailure("QuakeFaultHandler power cycled the output channel when QuakeManager was not disabled.")

        # Radio should power cycle if:
        #    - QFH is between forced_standby and powercycle_2 states before power cycling
        #    - Radio was set to disabled before power cycling
        #    - Cycles since blackout is at least as much as the time parameter
        should_powercycle = self.cycles_since_blackout_start >= time and self.is_radio_disabled() and self.is_powercycle_state()

        # Check second condition valid in HITL
        if (powercycled and not should_powercycle) or (should_powercycle and not powercycled):
            raise TestCaseFailure("QuakeFaultHandler failed to power cycle the output channel.")
       
    def diagnostics(self):
        self.read_state("radio.state")
        self.read_state("qfh.state")
        self.read_state("radio.last_comms_ccno")
        self.read_state("gomspace.power_cycle_output3_cmd")

    def run_case_singlesat(self):
        # The satellite has been in a blackout since startup. Cycle count starts at 1.
        self.cycles_since_blackout_start = self.rs("pan.cycle_no") - 1

        # These steps necessary in HOOTL, unsure about how it will affect HITL
        self.ws("radio.state", Enums.radio_states["wait"])
        self.ws("gomspace.power_cycle_output3_cmd",True)

        # Simulate one day of no comms
        self.logger.put(f"Creating a comms blackout of 24 hours, starting on control cycle: {self.rs('pan.cycle_no')}")
        self.advance_to_next_qfh_state(self.one_day_ccno)
            
        # Reset cycles
        self.cycles_since_blackout_start = 0

        # Simulate second day of no comms
        self.logger.put(f"Creating another comms blackout of 24 hours, starting on control cycle: {self.rs('pan.cycle_no')}")
        self.advance_to_next_qfh_state(self.one_day_ccno)
       
        # Proceed through all 3 power cycle stages, making sure QFH isn't power cycling in a non-wait state
        self.logger.put(f"Entering power cycle stages, starting on control cycle: {self.rs('pan.cycle_no')}")
        for i in range(3):
            # Reset cycles
            self.cycles_since_blackout_start = 0
            self.advance_to_next_qfh_state(self.one_day_ccno//3)


        self.logger.put("QuakeFaultHandler did not inappropriately power cycle when QuakeManager was in a non-wait state.")
        self.logger.put("Testcase finished.")

        self.finish()
