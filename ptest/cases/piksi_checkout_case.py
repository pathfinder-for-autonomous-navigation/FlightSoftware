# PiksiCheckoutCase. Checks the functionality of the Piksi
from .base import SingleSatCase
from .utils import Enums, mag_of, sum_of_differentials, TestCaseFailure
import math
    
class PiksiCheckoutCase(SingleSatCase):
    def __init__(self, *args, **kwargs):
        super(PiksiCheckoutCase, self).__init__(*args, **kwargs)

        self.debug_to_console = True
     
    def print_piksi_state(self):
        st = self.rs("piksi.state")
        self.logger.put(f"Piksi state is: {Enums.piksi_modes[st]}")

    def post_boot(self):
        self.print_header("Begin Piksi Checkout Case")

        self.n = 100

        self.vels = []
        self.positions = []
        self.baselines = []
        self.modes_dict = {x:0 for x in Enums.piksi_modes.arr}

        self.most_common_mode = Enums.piksi_modes["no_fix"]

        self.mission_state = "manual"

        # Just in case this is ever run in a HITL unit
        self.flight_controller.write_state("gomspace.piksi_off", False)

        # Needed so that PiksiControlTask updates its values
        for i in range(5):
            self.cycle()

    def check_vectors(self, name, vectors, mag_min, mag_max):

        self.print_header(f"CHECKING {name} VECTORS")

        for vec in vectors:
            mag = mag_of(vec)
            self.soft_assert((mag_min < mag and mag < mag_max),
                f"Piksi {name} reading out of expected bounds, Mag: {mag}")

        # check readings changed over time
        self.soft_assert(sum_of_differentials(vectors) > 0,
            f"Piksi {name} readings did not vary across readings.")

        self.print_header(f"CHECK {name} VECTORS COMPLETE")

    def nominal_checkout(self):
        self.print_header("ENTERING NOMINAL CHECKOUT")

        sync_errors = self.modes_dict["sync_error"]
        self.logger.put(f"Sync errors: {sync_errors}")
        self.soft_assert(sync_errors < self.n/10, f"Exceed 10% sync error rate: {sync_errors}")

        crc_errors = self.modes_dict["crc_error"]
        self.logger.put(f"CRC errors: {crc_errors}")
        self.soft_assert(crc_errors < self.n/10, f"Exceed 10% crc error rate: {crc_errors}")

        nsat_errors = self.modes_dict["nsat_error"]
        self.logger.put(f"Nsat errors: {nsat_errors}")
        self.soft_assert(nsat_errors == 0, f"NSAT ERRORS: {nsat_errors}")

        # no further checkouts apply
        if self.most_common_mode == 'no_fix' or self.most_common_mode == 'no_data_error':
            self.print_header("PIKSI LIKELY NO SIGNAL")
            return

        self.check_vectors("position", self.positions, (6371-10)*1000, (6371+10)*1000) # +-10 km above the surface of earth in m
        self.check_vectors("velocity", self.vels, 0, 5000) # simulator speed is 4000 millilmeters per second

        if self.most_common_mode == 'spp':
            self.print_header("PIKSI GPS SIGNAL BUT NO RTK")    
            return

        self.check_vectors("baseline", self.baselines, 0, 105*1000) # simulator max distance is 100 meters
        self.print_header("PIKSI RTK")    

    def run(self):

        # Take 10 readings just for observation
        self.print_header("10 Readings for observation: ")

        for i in range(10):
            self.cycle()

            m = self.print_rs("piksi.state")
            self.logger.put(f"Mode: {Enums.piksi_modes[m]}")
            v = self.print_rs("piksi.vel")
            self.logger.put(f"VEL MAG: {mag_of(v)}")
            p = self.print_rs("piksi.pos")
            self.logger.put(f"POS MAG: {mag_of(p)}")
            b = self.print_rs("piksi.baseline_pos")
            self.logger.put(f"BASELINE MAG: {mag_of(b)}")
            d = self.print_rs("piksi.microdelta")
            self.logger.put("")

        # Take N readings for actual analysis
        self.print_header(f"TAKING {self.n} PIKSI READINGS FOR ANALYSIS")

        # Catalog readings
        for i in range(self.n):
            self.cycle()

            mode = self.rs("piksi.state")
            self.modes_dict[Enums.piksi_modes[mode]] += 1

            self.vels += [self.rs("piksi.vel")]
            self.positions += [self.rs("piksi.pos")]
            self.baselines += [self.rs("piksi.baseline_pos")]

        mode_rank = [x for x in Enums.piksi_modes.arr] # 11 total piksi modes
        mode_rank = sorted(mode_rank, key = self.modes_dict.get, reverse = True)

        self.most_common_mode = mode_rank[0]
        self.second_most_common_mode = mode_rank[1]

        first_num = self.modes_dict[self.most_common_mode]
        second_num = self.modes_dict[self.second_most_common_mode]

        self.print_header(f"MOST COMMON MODE: {self.most_common_mode} @ {first_num} readings.")
        self.print_header(f"SECOND MOST COMMON: {self.second_most_common_mode} @ {second_num} readings.")

        self.logger.put("Readings Dictionary: ")
        self.logger.put(str(self.modes_dict))

        nominal_list = ["spp","fixed_rtk","float_rtk", "no_fix"]
        raise_fail_list = ["sync_error", "nsat_error", "crc_error", "time_limit_error", "data_error", "no_data_error"]

        # When in no fix, no_data_error is actually the most common mode
        if self.most_common_mode == 'no_data_error' and self.second_most_common_mode == 'no_fix':
            self.nominal_checkout()
        elif self.most_common_mode in nominal_list:
            self.nominal_checkout()
        elif self.most_common_mode in raise_fail_list:
            raise TestCaseFailure(f"{self.most_common_mode} was the most common mode. Not Nominal")

        # Raise an error if got an unrecognized mode as most common
        else:
            raise TestCaseFailure("MISCONFIGURED PTEST CASE")

        self.print_header("PIKSI CHECKOUT COMPLETE")
        self.finish()
