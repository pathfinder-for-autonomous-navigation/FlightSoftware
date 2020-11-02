# Empty test case. Gets cycle count purely for diagnostic purposes
from .base import SingleSatOnlyCase
from .utils import Enums, TestCaseFailure


class PowerLossCheckoutCase(SingleSatOnlyCase):
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

    def str_to_bool(self, string):
            if string == "true":
                return True
            elif string == "false":
                return False
            else:
                print(string)
                raise ValueError

    def check_powercycle(self):
        return self.str_to_bool(self.read_state("gomspace.power_cycle_output3_cmd"))

    def radio_in_wait(self):
        return self.rs("radio.state") == 0 #Enums.radio_states["wait"] equals 1?

    def check_faulty_powercycle(self,bound,radioBeenInWait):
        check_powercycle = self.check_powercycle()
        qfh_state = int(self.read_state("qfh.state"))
        
        # If radio is not in wait, QFH should not power cycle
        if not self.radio_in_wait() and check_powercycle:
            raise TestCaseFailure("QuakeFaultHandler power cycled the output channel when QuakeManager was not in wait.")
        
        # Radio should power cycle if:
        #    - QFH is between forced_standby and powercycle_2 states before powercycling
        #    - Radio was in wait in the previous cycle and is currently in wait 
        #      (unsure if it can be in wait for longer than this before power cycling, someone pls check this condition)
        #    - Cycles since blackout is at least as much as the bound
        should_powercycle = self.cycles_since_blackout_start >= bound and self.radio_in_wait() and radioBeenInWait and (2 <= qfh_state <= 4)
        if (should_powercycle and not check_powercycle) or (check_powercycle and not should_powercycle):
            raise TestCaseFailure("QuakeFaultHandler failed to power cycle the output channel when QuakeManager in wait.")

    def diagnostics(self):
        self.read_state("radio.state")
        self.read_state("qfh.state")
        self.read_state("radio.last_comms_ccno")
        self.read_state("gomspace.power_cycle_output3_cmd")

    def run_case_singlesat(self):
        self.failed = False

        # The satellite has been in a blackout since startup. Cycle count starts at 1.
        self.cycles_since_blackout_start = self.rs("pan.cycle_no") - 1
        self.radio_in_wait()

        # Simulate one day of no comms
        self.logger.put(f"Creating a comms blackout of 24 hours, starting on control cycle: {self.rs('pan.cycle_no')}")
        while self.cycles_since_blackout_start < self.one_day_ccno:
            radioWasInWait = self.radio_in_wait()
            self.cycle()
            self.diagnostics()
            self.cycles_since_blackout_start += 1
            self.check_faulty_powercycle(self.one_day_ccno,radioWasInWait)
            
        # Reset cycles
        self.cycles_since_blackout_start = 0

        # Simulate second day of no comms
        self.logger.put(f"Creating another comms blackout of 24 hours, starting on control cycle: {self.rs('pan.cycle_no')}")
        while True:
            # Check that radio was already in wait before cycling. QFH cannot power cycle in the same cycle radio switches to wait state.
            # Radio must already be in wait in order to power cycle.
            radioWasInWait = self.radio_in_wait()

            self.cycle()
            self.diagnostics()
            self.cycles_since_blackout_start += 1
            self.check_faulty_powercycle(self.one_day_ccno,radioWasInWait)

            if (self.cycles_since_blackout_start >= self.one_day_ccno) and self.radio_in_wait() and radioWasInWait:
                break
       
        # Reset cycles
        self.cycles_since_blackout_start = 0

        # Proceed through all 3 power cycle stages, making sure QFH isn't power cycling in a non-wait state
        self.logger.put(f"Entering power cycle stages, starting on control cycle: {self.rs('pan.cycle_no')}")
        for i in range(3):
            # Reset cycles
            self.cycles_since_blackout_start = 0
            while True:
                # Check that radio was in wait before cycling.
                radioWasInWait = self.radio_in_wait()
                self.cycle()
                self.diagnostics()
                self.cycles_since_blackout_start += 1
                self.check_faulty_powercycle(self.one_day_ccno/3,radioWasInWait)

                if (self.cycles_since_blackout_start >= self.one_day_ccno/3) and self.radio_in_wait() and radioWasInWait:
                    break


        self.logger.put("QuakeFaultHandler did not inappropriately power cycle when QuakeManager was in a non-wait state.")
        self.logger.put("Testcase finished.")

        self.finish()
