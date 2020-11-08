# Empty test case. Gets cycle count purely for diagnostic purposes
from .base import SingleSatOnlyCase
from .utils import Enums, TestCaseFailure


class QuakePowercycling(SingleSatOnlyCase):
    @property
    def sim_duration(self):
        return float("inf")

    @property
    def desired_initial_state(self):
        return "standby"

    @property
    def fast_boot(self):
        return False

    def setup_post_bootsetup(self):
        self.ws("fault_handler.enabled", True)

    def check_powercycle(self):
        return self.rs("gomspace.power_cycle_output3_cmd")

    def is_radio_in_wait(self):
        return self.rs("radio.state") == Enums.radio_states["wait"] 

    def is_powercycle_state(self):
        return self.rs("qfh.state") in [Enums.qfh_states["powercycle_1"],Enums.qfh_states["powercycle_2"] ,Enums.qfh_states["powercycle_3"]]

    def advance_to_next_qfh_state(self,time):
        while True:
            # Check that radio was already in wait before cycling. QFH cannot power cycle in the same cycle radio switches to wait state.
            # Radio must already be in wait in order to power cycle.
            radioWasInWait = self.is_radio_in_wait()

            self.cycle()
            self.diagnostics()
            self.cycles_since_blackout_start += 1
            self.check_correct_powercycle_behavior(time,radioWasInWait)

            if (self.cycles_since_blackout_start >= time) and self.is_radio_in_wait() and radioWasInWait:
                break


    def check_correct_powercycle_behavior(self,time,radioBeenInWait):
        powercycled = self.check_powercycle()
        
        # If radio is not in wait, QFH should not power cycle
        if not self.is_radio_in_wait() and powercycled:
            raise TestCaseFailure("QuakeFaultHandler power cycled the output channel when QuakeManager was not in wait.")
        
        # Radio should power cycle if:
        #    - QFH is between forced_standby and powercycle_2 states before powercycling
        #    - Radio was in wait in the previous cycle and is currently in wait 
        #      (unsure if it can be in wait for longer than this before power cycling, someone pls check this condition)
        #    - Cycles since blackout is at least as much as the time
        should_powercycle = self.cycles_since_blackout_start >= time and self.is_radio_in_wait() and radioBeenInWait and self.is_powercycle_state()
        if (powercycled and not should_powercycle): # unsure whether (should_powercycle and not powercycled) is valid condition
            raise TestCaseFailure("QuakeFaultHandler failed to power cycle the output channel when QuakeManager in wait.")

    def diagnostics(self):
        self.read_state("radio.state")
        self.read_state("qfh.state")
        self.read_state("radio.last_comms_ccno")
        self.read_state("gomspace.power_cycle_output3_cmd")

    def run_case_singlesat(self):

        # The satellite has been in a blackout since startup. Cycle count starts at 1.
        self.cycles_since_blackout_start = self.rs("pan.cycle_no") - 1
        self.is_radio_in_wait()

        # Simulate one day of no comms
        self.logger.put(f"Creating a comms blackout of 24 hours, starting on control cycle: {self.rs('pan.cycle_no')}")
        while self.cycles_since_blackout_start < self.one_day_ccno:
            radioWasInWait = self.is_radio_in_wait()
            self.cycle()
            self.diagnostics()
            self.cycles_since_blackout_start += 1
            self.check_correct_powercycle_behavior(self.one_day_ccno,radioWasInWait)
            
        # Reset cycles
        self.cycles_since_blackout_start = 0

        # Simulate second day of no comms
        self.logger.put(f"Creating another comms blackout of 24 hours, starting on control cycle: {self.rs('pan.cycle_no')}")
        self.advance_to_next_qfh_state(self.one_day_ccno)
       
        # Reset cycles
        self.cycles_since_blackout_start = 0

        # Proceed through all 3 power cycle stages, making sure QFH isn't power cycling in a non-wait state
        self.logger.put(f"Entering power cycle stages, starting on control cycle: {self.rs('pan.cycle_no')}")
        for i in range(3):
            # Reset cycles
            self.cycles_since_blackout_start = 0
            self.advance_to_next_qfh_state(self.one_day_ccno/3)


        self.logger.put("QuakeFaultHandler did not inappropriately power cycle when QuakeManager was in a non-wait state.")
        self.logger.put("Testcase finished.")

        self.finish()
