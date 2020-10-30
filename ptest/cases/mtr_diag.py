# ADCSCheckoutCase. Verifies the functionality of the ADCS.
from .base import SingleSatOnlyCase
from .utils import Enums, mag_of, sum_of_differentials
import math
import time

class MTRDiag(SingleSatOnlyCase):
    def assert_vec_within(self, expected, actual, delta):
        assert(len(expected) == len(actual))
        length = len(expected)

        for i in range(length):
            self.soft_assert(abs(expected[i]-actual[i]) < delta, 
                f"Element #{i}, Expected {expected[i]}, got {actual[i]}. Diff exceed delta of {delta}.")

    def setup_post_bootsetup(self):
        self.print_header("Begin ADCS Checkout Case")

        self.ws("cycle.auto", False)

        # Needed so that ADCSMonitor updates its values
        self.cycle()
        self.ws("dcdc.ADCSMotor_cmd", True)

        # Necessary so that motor commands are pre-empted by ADCS DCDC being on
        self.cycle()

        self.print_ws("adcs.state", Enums.adcs_states["manual"])
        self.print_ws("adcs_cmd.mtr_mode", Enums.mtr_modes["MTR_ENABLED"])

        self.cycle()

        self.print_header("Finished Initialization")

    
    def run_case_singlesat(self):
        # write the command and read the command and verify a correct relaying.
        self.print_header("Verify Command Write and Command Read")

        mtr_max = 0.05666
        command1 = [mtr_max, mtr_max, -mtr_max]
        self.print_ws("adcs_cmd.mtr_cmd", command1)
        self.cycle()
        read_cmd = self.print_rs("adcs_cmd.mtr_cmd")
        self.assert_vec_within(command1, read_cmd, 1e-6)

        while(True):
            time.sleep(1)

        self.finish()
        pass