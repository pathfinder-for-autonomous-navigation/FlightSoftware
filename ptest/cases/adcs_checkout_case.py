# ADCSCheckoutCase. Verifies the functionality of the ADCS.
from .base import SingleSatOnlyCase
import math

def mag_of(vals):
    assert(isinstance(vals, list))
    return math.sqrt(sum([x*x for x in vals]))

def sum_of_differentials(lists_of_vals):
    total_diff = [0 for x in lists_of_vals[0]]
    for i in range(len(lists_of_vals) - 1):
        diff = [abs(lists_of_vals[i][j] - lists_of_vals[i+1][j]) for j in range(len(lists_of_vals[i]))]
        total_diff = [diff[x] + total_diff[x] for x in range(len(total_diff))]

    return sum(total_diff)
class ADCSCheckoutCase(SingleSatOnlyCase):

    @property
    def havt_read(self):
        read_list = [False for x in range(self.havt_length)]
        for x in range(self.havt_length):
            read_list[x] = self.rs("adcs_monitor.havt_device"+str(x))
        return read_list

    def step(self):
        ''' 
        Steps the FC forward by one CC

        Asserts the FC did indeed step forward by one CC
        '''
        init = self.rs("pan.cycle_no")
        self.sim.flight_controller.write_state("cycle.start", "true")
        assert(self.rs("pan.cycle_no") == init + 1), f"FC did not step forward by one cycle"

    def print_havt_read(self):
        binary_list = [1 if x else 0 for x in self.havt_read]

        string_of_binary_list = [str(x) for x in binary_list]
        
        # Reverse the list so it prints as it does in ADCSSoftware
        string_of_binary_list.reverse()

        list_of_list = [string_of_binary_list[4*i:(4*i)+4] for i in range((int)(self.havt_length/4)+1)]
        final = [x + [" "] for x in list_of_list]

        final_string = ''.join([''.join(x) for x in final])
        print("HAVT Read: "+str(final_string))

    def run_case_singlesat(self):

        self.print_header("Begin ADCS Checkout Case")

        # Needed so that ADCSMonitor updates its values
        self.step()

        self.print_rs("adcs_monitor.functional")
        assert(self.rs("adcs_monitor.functional")), f"ADCSC Not Functional"

        self.ws("pan.state", 11)
        self.ws("adcs.state", 5)
        self.ws("adcs_cmd.rwa_mode", 1)
        self.ws("adcs_cmd.rwa_speed_cmd", [0,0,0])
        self.ws("dcdc.ADCSMotor_cmd", True)

        # reset all devices in case last ptest case left an "unclean state"
        for x in range(self.havt_length):
            self.ws(f"adcs_cmd.havt_reset{x}", True)

        # adcs_controller should have applied commands.
        self.step()

        self.print_havt_read()

        for x in range(self.havt_length):
            if not self.havt_read[x]:
                print(f"Device #{x}, {self.havt_devices.get_by_num(x)} is not functional")

        self.print_header("Finished Initialization")

        print("Initial HAVT Table:")
        self.print_havt_read()

        # Note IMUGYR on left
        # Just FC and ADCSC
        barebones_hitl = "000111011100" + "000000"

        # FC + ADCSC + MAG1 + MAG2 + GYR
        minimal_hitl   = "111111011100" + "000000"
            
        # I forgot what it actually is, will update on next PR, after testing with EDU SAT
        edu_sat        = "110111011100" + "000000"

        test_beds = {barebones_hitl:"BAREBONES HITL", minimal_hitl:"MINIMAL HITL", edu_sat:"EDU SAT"}

        binary_string_havt_read = ''.join(["1" if x else "0" for x in self.havt_read])

        if(binary_string_havt_read not in test_beds):
            self.print_header("UN-RECOGNIZED HITL TEST BED. MAKE SURE HAVT_READ IS AS EXPECTED!")
        else:
            self.print_header(f"HAVT TABLE MATCHES WITH: {test_beds[binary_string_havt_read]}")
            
        # cache the initially functional devices
        initial_up_devices = self.havt_read

        # disable all devices
        for x in range(self.havt_length):
            self.ws(f"adcs_cmd.havt_disable{x}", True)

        self.step()

        print("Post disabling all devices:")
        self.print_havt_read()
        self.soft_assert(([0 for x in range(self.havt_length)] == self.havt_read), 
            "Disabling all devices failed")

        # reset all devices
        for x in range(self.havt_length):
            self.ws(f"adcs_cmd.havt_reset{x}", True)
        self.step()
        print("Post resetting all devices:")
        self.print_havt_read()
        self.soft_assert((initial_up_devices == self.havt_read), 
            "Disable Reset Cycle Failed, New HAVT Table does not match initial table cache")

        print("Reset-Disable Success. All initially functional devices remain functional.")
        
        # BEGIN SENSOR CHECKOUT
        self.print_header("Begin Sensor Checkout")
        # expect each sensor value to change from cycle to cycle, given user is jostling the test bed.

        # If either mag1 or mag2 are up, run a check on it.
        if self.rs("adcs_monitor.havt_device1") or self.rs("adcs_monitor.havt_device2"):

            # TODO make section compatible with 2x imu active (LATER)
            self.print_header("Begin MAG Checkout")
            
            self.print_rs("adcs_cmd.imu_mode")
            imu_mode = self.rs("adcs_cmd.imu_mode")
            imu_modes = ["MAG1 active", "MAG2 active", "MAG1 calibrate", "MAG2 calibrate"]
            print(f"IMU Mode: {imu_modes[imu_mode]}")
            
            # perform 10 readings.
            list_of_mag_rds = []
            for i in range(10):
                self.step()
                list_of_mag_rds += [self.rs("adcs_monitor.mag_vec")]

            # for each reading check, magnitude bounds
            # earth's mag field is between 25 to 65 microteslas - Wikipedia
            print("Mag readings: ")
            for i in range(10):
                mag = mag_of(list_of_mag_rds[i])
                print(f"{list_of_mag_rds[i]}, mag: {mag}")
                self.soft_assert((25e-6 < mag and mag < 65e-6),
                    "Mag reading out of expected (earth) bounds.")

            # check readings changed over time
            self.soft_assert(sum_of_differentials(list_of_mag_rds) > 0,
                "Mag readings did not vary across readings.")

            self.print_header("MAG CHECKOUT COMPLETE")

        # Run checks on GYR if GYR is up.
        if self.rs("adcs_monitor.havt_device0"):
            self.print_header("Begin GYR Checkout")
            
            # perform 10 readings.
            list_of_gyr_rds = []
            for i in range(10):
                self.step()
                list_of_gyr_rds += [self.rs("adcs_monitor.gyr_vec")]

            # for each reading check, magnitude bounds
            # expected rotation???
            print("GYR readings: ")
            for i in range(10):
                mag = mag_of(list_of_gyr_rds[i])
                print(f"{list_of_gyr_rds[i]}, mag: {mag}")
                # 3.8 ~= sqrt((125 * 0.017)^2 * 3)
                self.soft_assert((0 < mag and mag < 3.8),
                    "Gyr reading out of expected bounds.")

            # check readings changed over time
            self.soft_assert(sum_of_differentials(list_of_gyr_rds) > 0,
                "Gyr readings did not vary across readings.")

            self.print_header("GYR CHECKOUT COMPLETE")

        # TODO FURTHER CHECKOUTS

        self.print_header("ADCS CHECKOUT COMPLETE")