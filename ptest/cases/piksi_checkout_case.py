# PiksiCheckoutCase. Checks the functionality of the Piksi
from .base import SingleSatOnlyCase, TestCaseFailure
import math
    
class PiksiCheckoutCase(SingleSatOnlyCase):

    @property
    def havt_read(self):
        read_list = [False for x in range(self.havt_length)]
        for x in range(self.havt_length):
            read_list[x] = self.rs("adcs_monitor.havt_device"+str(x))
        return read_list

    def print_piksi_state(self):
        st = self.rs("piksi.state")
        self.logger.put(f"Piksi state is: {self.piksi_modes.get_by_num(st)}")

    def setup_case_singlesat(self):
        self.print_header("Begin Piksi Checkout Case")

        self.determined_mode = self.piksi_modes.get_by_name("no_fix")

        # Needed so that PiksiControlTask updates its values
        for i in range(5):
            self.cycle()

        self.ws("pan.state", self.mission_states.get_by_name("manual"))

    def determine_mode(self):
        '''
        Determines the probable simulation or actual state of the Piksi

        Returns a Piksi Mode
        '''
        '''
        perform 10 readings,
        return the most common lol
        '''

        self.log.put(self.piksi_modes.arr)

        readings_dict = {x:0 for x in self.piksi_modes.arr}

        readings = []
        for i in range(20):
            self.cycle()
            reading = self.rs("piksi.state")
            readings += [self.rs("piksi.state")]
            readings_dict[reading] += 1

        #if dead is the most common/throw a soft assertion error

    def fixed_rtk_checkout(self):
        raise NotImplementedError

    def float_rtk_checkout(self):
        raise NotImplementedError

    def spp_checkout(self):
        raise NotImplementedError

    def no_fix_checkout(self):
        raise NotImplementedError

    def run_case_singlesat(self):
        
        self.print_rs("piksi.state")

        for i in range(10):
            self.cycle()
            self.print_piksi_state()

        list_of_pos_rds = []
        for i in range(10):
            self.cycle()
            pos = self.rs("piksi.pos")
            mag = self.mag_of(pos)
            list_of_pos_rds += [pos] 
            
            # TODO FIND EXPECTED BOUNDS
            self.soft_assert((0 < mag and mag < 1e8),
                "Piksi position reading out of expected bounds.")

        # check readings changed over time
        self.soft_assert(self.sum_of_differentials(list_of_pos_rds) > 0,
            "Piksi position readings did not vary across readings.")

        for i in range(10):
            self.cycle()
            self.print_piksi_state()
            self.print_rs("piksi.pos")

        # TODO FURTHER CHECKOUTS

        self.print_header("PIKSI CHECKOUT COMPLETE")
        self.finish()