# Empty test case. Gets cycle count purely for diagnostic purposes
from .base import SingleSatOnlyCase
from .utils import Enums


class PowerLossCheckoutCase(SingleSatOnlyCase):
    @property
    def sim_duration(self):
        return float("inf")

    @property
    def desired_initial_state(self):
        return "standby"

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

    # QFH should want to power cycle radio, but if QM is not in wait, it should not power cycle
    def check_faulty_powercycle(self):
        check_powercycle = self.check_powercycle()
        radio_state = int(self.read_state("radio.state"))
        if radio_state > 0 and check_powercycle:
            self.failed = True
            self.logger.put("QuakeFaultHandler power cycled the output channel when QuakeManager was not in wait.")
        if radio_state == 0 and not check_powercycle:
            self.failed = True
            self.logger.put("QuakeFaultHandler failed to power cycle the output channel when QuakeManager in wait.")

    # What seems to be happening at least in the instances I've run this is
    # radio state is always > 0 by the time we are checking for power cycle
    # so it's just never power cycling.

    # Also, this run case seems to run for a single value of radio state. Maybe
    # want to increase the number of cycles so this is running for different
    # radio states? Might have to run this having set QM at various different
    # starting states?

    def diagnostics(self):
        self.read_state("radio.state")
        self.read_state("qfh.state")
        self.read_state("radio.last_comms_ccno")
        self.read_state("gomspace.power_cycle_output3_cmd")
        self.read_state("gomspace.output.output3")

    def run_case_singlesat(self):
        self.failed = False

        self.write_state("radio.state", Enums.radio_states["wait"])

        # Check that if QM is a non-wait state and QFH wants to powercycle the 
        # radio, QFH is unable to until QM returns to a wait state

        # The satellite has been in a blackout since startup.
        #
        # Note: we subtract 1 because the control cycle count starts at 1.
        # So if the current ccno is 1, that implies there have been zero
        # cycles since the blackout started.
        self.cycles_since_blackout_start = self.rs("pan.cycle_no") - 1

        # Simulate one day of no comms
        self.logger.put(f"Creating a comms blackout of 24 hours (measured from control cycle #1.) Current control cycle: {self.rs('pan.cycle_no')}")
        while self.cycles_since_blackout_start < self.one_day_ccno:
            self.cycle()
            self.diagnostics()
            self.cycles_since_blackout_start += 1
            
            #pug = self.read_state("qfh.state")
            #self.logger.put(f"qfh state: {pug}")
        
        # Reset cycles
        self.cycles_since_blackout_start = 0

        last_qfh_state = self.read_state("qfh.state")

        # Simulate second day of no comms
        self.logger.put(f"Creating another comms blackout of 24 hours, starting on control cycle {self.rs('pan.cycle_no')}")
        while self.cycles_since_blackout_start < self.one_day_ccno:
            self.cycle()
            

            curr_cycle = self.rs("pan.cycle_no")
            curr_qfh_state = self.read_state("qfh.state")
            if curr_qfh_state > last_qfh_state:
                powercycle_flag = self.check_powercycle()
                self.logger.put(f"qfh state incremented on cycle {curr_cycle}")
                self.logger.put(f"qfh state incremented {curr_qfh_state}")
                self.logger.put(f"powercycle flag was set to {powercycle_flag}")

            last_qfh_state = curr_qfh_state


            self.diagnostics()
            self.cycles_since_blackout_start += 1


        # After second day of no comms, qfh should powercycle to 1st PC state
        curr_cycle = self.rs("pan.cycle_no")
        qfh_state = self.read_state("qfh.state")
        self.logger.put(f"Second day of no comms ends on cycle {curr_cycle}")
        self.logger.put(f"QFH state after second day is {qfh_state}")

        # Qfh should want to powercycle into state powercycle_1
        self.check_faulty_powercycle()
       

        self.powercycles_count = 0

        # Proceed through all 3 power cycle stages, making sure QFH isn't power cycling in a non-wait state
        for x in range(3):
            # Reset cycles
            self.cycles_since_blackout_start = 0
            self.logger.put(f"Power cycle round  {x+1}:")

            curr_cycle = self.rs("pan.cycle_no")
            qfh_state = self.read_state("qfh.state")
            self.logger.put(f"1/3 day of powercycle state ends on cycle {curr_cycle}")
            self.logger.put(f"QFH state after 1/3 day is {qfh_state}")

            # Cycle until QFH is ready to powercycle
            while self.cycles_since_blackout_start < self.one_day_ccno / 3:
                self.cycle()
                self.diagnostics()
                self.cycles_since_blackout_start += 1

                curr_cycle = self.rs("pan.cycle_no")
                curr_qfh_state = self.read_state("qfh.state")
                if curr_qfh_state > last_qfh_state:
                    powercycle_flag = self.check_powercycle()
                    self.logger.put(f"qfh state incremented on cycle {curr_cycle}")
                    self.logger.put(f"qfh state incremented {curr_qfh_state}")
                    self.logger.put(f"powercycle flag was set to {powercycle_flag}")

                last_qfh_state = curr_qfh_state
            

            # Debugging
            # self.logger.put(f"Power cycle round  {x+1}:")
            # self.logger.put(f"Radio state is {self.read_state('radio.state')}")
            # self.logger.put(f"QFH state is {self.read_state('qfh.state')}")

            # Qfh should want to powercycle
            self.check_faulty_powercycle()

        
            
            
        if not self.failed:
            self.logger.put("QuakeFaultHandler did not inappropriately power cycle when QuakeManager was in a non-wait state.")
        else:
            self.logger.put("Powercycling behavior did not match expected behavior.")
        self.logger.put("Testcase finished.")

        self.finish()


