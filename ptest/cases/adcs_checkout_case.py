# ADCSCheckoutCase. Verifies the functionality of the ADCS.
from .base import SingleSatCase
from .utils import Enums, mag_of, sum_of_differentials
import math
import time

def abs_of(list_of_vals):
    '''
    Given a list of values, take the absolute value of each value
    '''
    return [abs(x) for x in list_of_vals]

def list_of_avgs(lists_of_vals):
    '''
    Given a list of lists of vals, return a list of the average value of each list.
    '''
    sum_of_each = [sum(x) for x in lists_of_vals]
    len_of_each = len(lists_of_vals[0])
    return [sum_of_each[i]/len_of_each for i in lists_of_vals]

class ADCSCheckoutCase(SingleSatCase):
    def assert_vec_within(self, expected, actual, delta):
        assert(len(expected) == len(actual))
        length = len(expected)

        for i in range(length):
            self.soft_assert(abs(expected[i]-actual[i]) < delta, 
                f"Element #{i}, Expected {expected[i]}, got {actual[i]}. Diff exceed delta of {delta}.")

    def post_boot(self):
        self.ws("pan.state", Enums.mission_states["manual"])
        self.print_header("Begin ADCS Checkout Case")

        self.ws("cycle.auto", False)

        # Needed so that ADCSMonitor updates its values
        self.cycle()
        self.ws("dcdc.ADCSMotor_cmd", True)

        # Necessary so that motor commands are pre-empted by ADCS DCDC being on
        self.cycle()

        self.ws("adcs.state", Enums.adcs_states["manual"])
        self.ws("adcs_cmd.rwa_mode", Enums.rwa_modes["RWA_SPEED_CTRL"])
        self.ws("adcs_cmd.rwa_speed_cmd", [0,0,0])
    
        self.print_header("Finished Initialization")

    def havt_checkout(self):
        # reset all devices in case last ptest case left an "unclean state"
        for x in range(Enums.havt_length):
            self.ws(f"adcs_cmd.havt_reset{x}", True)

        # adcs_controller should have applied commands.
        self.cycle()

        self.print_havt_read()

        self.print_non_functional_adcs_havt()
        
        self.logger.put("Initial HAVT Table:")
        self.print_havt_read()

        # Note IMUGYR on left
        # GYR_HEATER on right, default off
        # FC only
        fc_only_hitl   = "000000000000" + "0000000"

        # Just FC and ADCSC
        barebones_hitl = "000111011100" + "0000001"

        # FC + ADCSC + MAG1 + MAG2 + GYR
        minimal_hitl   = "111111011100" + "0000001"
            
        # ONLY MAG 2 IS DOWN ON EDU SAT as of 04/06/2020
        edu_sat        = "110111111111" + "1111111"

        test_beds = {fc_only_hitl:"FC ONLY HITL",barebones_hitl:"BAREBONES HITL", minimal_hitl:"MINIMAL HITL", edu_sat:"EDU SAT"}

        binary_string_havt_read = ''.join(["1" if x else "0" for x in self.havt_read])

        if(binary_string_havt_read not in test_beds):
            self.print_header("UN-RECOGNIZED HITL TEST BED. MAKE SURE HAVT_READ IS AS EXPECTED!")
        else:
            self.print_header(f"HAVT TABLE MATCHES WITH: {test_beds[binary_string_havt_read]}")

        # cache the initially functional devices
        initial_up_devices = self.havt_read

        # disable all devices
        for x in range(Enums.havt_length):
            self.ws(f"adcs_cmd.havt_disable{x}", True)

        self.cycle()

        self.logger.put("Post disabling all devices:")
        self.print_havt_read()
        self.soft_assert(all([x == 0 for x in self.havt_read]), 
            "Disabling all devices failed")

        # reset all devices
        for x in range(Enums.havt_length):
            self.ws(f"adcs_cmd.havt_reset{x}", True)
        self.cycle()
        self.logger.put("Post resetting all devices:")
        self.print_havt_read()
        self.soft_assert((initial_up_devices == self.havt_read), 
            "Disable Reset Cycle Failed, New HAVT Table does not match initial table cache",
            "Reset-Disable Success. All initially functional devices remain functional.")

    def mag_checkout(self, mag_num):
        '''
        Run a checkout case on a given magnetometer
        '''
        assert(mag_num in [1, 2])
        self.print_header(f"Begin MAG{mag_num} Checkout")
        
        self.print_rs(f"adcs_cmd.mag{mag_num}_mode")
        imu_mode = self.rs(f"adcs_cmd.mag{mag_num}_mode")
        self.logger.put(f"MAG{mag_num} Mode: {Enums.imu_modes[imu_mode]}")

        # perform 10 readings.
        list_of_mag_rds = []
        for i in range(10):
            self.cycle()
            list_of_mag_rds += [self.rs(f"adcs_monitor.mag{mag_num}_vec")]

        # for each reading check, magnitude bounds
        # earth's mag field is between 25 to 65 microteslas - Wikipedia
        self.logger.put(f"MAG{mag_num} readings: ")
        for i in range(10):
            mag = mag_of(list_of_mag_rds[i])
            self.logger.put(f"{list_of_mag_rds[i]}, mag: {mag}")
            self.soft_assert((25e-6 < mag and mag < 65e-6),
                f"MAG{mag_num} reading out of expected (earth) bounds.")

        # check readings changed over time
        self.soft_assert(sum_of_differentials(list_of_mag_rds) > 0,
            f"MAG{mag_num} readings did not vary across readings.")

        self.print_header(f"MAG{mag_num} CHECKOUT COMPLETE")

    def mag_independence_checkout(self):
        '''
        Run checks to make one mag work when the other of them is non functional
        '''

        self.print_header("MAG INDEPENDENCE CHECKOUT COMPLETE")

        # Disable MAG1
        self.ws("adcs_cmd.havt_disable1", True)
        self.cycle()
        self.logger.put("MAG1 DISABLED")
        self.mag_checkout(2)

        # Renable MAG1
        self.ws("adcs_cmd.havt_reset1", True)
        self.cycle()
        self.logger.put("MAG1 RESET")

        # Disable MAG2
        self.ws("adcs_cmd.havt_disable2", True)
        self.cycle()
        self.logger.put("MAG2 DISABLED")
        self.mag_checkout(1)        

        # Renable MAG2
        self.ws("adcs_cmd.havt_reset2", True)
        self.cycle()
        self.logger.put("MAG2 RESET")

        self.print_header("MAG INDEPENDENCE CHECKOUT COMPLETE")

    def gyr_checkout(self):
        self.print_header("Begin GYR Checkout")
            
        # perform 10 readings.
        list_of_gyr_rds = []
        for i in range(10):
            self.cycle()
            list_of_gyr_rds += [self.rs("adcs_monitor.gyr_vec")]

        # for each reading check, magnitude bounds
        self.logger.put("GYR readings: ")
        for i in range(10):
            mag = mag_of(list_of_gyr_rds[i])
            self.logger.put(f"{list_of_gyr_rds[i]}, mag: {mag}")
            # 3.8 is approximately the maximum possible magnitude GYR reading.
            # 125 * 0.017 = max_rd_omega
            # 3.8 ~= sqrt((125 * 0.017)^2 * 3)
            self.soft_assert((0 < mag and mag < 3.8),
                "Gyr reading out of expected bounds.")

        # check readings changed over time
        self.soft_assert(sum_of_differentials(list_of_gyr_rds) > 0,
            "Gyr readings did not vary across readings.")

        self.print_header("GYR CHECKOUT COMPLETE")

    def wheel_checkout(self):
        '''
        Check that wheels respond to speed and torque tests as expected
        '''
        self.print_header("BEGIN WHEEL CHECKOUT")
        
        self.logger.put("Checking RWA POT functionality:")

        self.ws("cycle.auto", True)

        # The below section checks that wheel speed readings change over time 
        # and are of expected magnitude
        self.ws("adcs_cmd.rwa_speed_cmd", [10, 10, 10])
        time.sleep(.2)
        self.print_rs("adcs_monitor.rwa_speed_rd")

        # perform 10 readings.
        list_of_speed_rds = []
        for i in range(10):
            cn = self.rs("pan.cycle_no")
            reading = [self.rs("adcs_monitor.rwa_speed_rd")]
            list_of_speed_rds += reading
            self.logger.put(f"Cycle No: {cn}, Speed Vec: {reading}")
            
        # check readings changed over time
        self.soft_assert(sum_of_differentials(list_of_speed_rds) > 0,
            "Pot readings did not vary across readings.")        

        reading = self.rs("adcs_monitor.rwa_speed_rd")
        self.assert_vec_within([10, 10, 10], reading, 4)

        # Begin Stress Testing Various Speeds

        wheel_speed_tests = [
            [20,40,50],
            [100,200,300],
            [-100,100,-40],
            [100,50,10],
            [500,500,500],
            [680,680,680], 
            [-680,-680,-680]
        ]

        for cmd_array in wheel_speed_tests:
            self.print_rs("gomspace.vbatt")
            self.logger.put("ADCS_CMD TARGET SPEED: ")
            self.print_ws("adcs_cmd.rwa_speed_cmd", cmd_array)
            time.sleep(5)
            reading = self.print_rs("adcs_monitor.rwa_speed_rd")
            self.assert_vec_within(cmd_array, reading, 1)
            time.sleep(1)

        # "Stop" Wheels
        self.ws("adcs_cmd.rwa_speed_cmd", [0,0,0])
        time.sleep(1)

        # Begin Torque Stress Test

        torque_max = 0.00418
        x = 0.0001
        y = 0.002
        z = torque_max
        torque_tests = [
            [x, 0, 0],
            [0, x, 0],
            [0, 0, x],
            [-x, 0, 0],
            [0, -x, 0],
            [0, 0, -x],
            [y, 0, 0],
            [0, y, 0],
            [0, 0, y],
            [-y, 0, 0],
            [0, -y, 0],
            [0, 0, -y],
            [z, 0, 0],
            [0, z, 0],
            [0, 0, z],
            [-z, 0, 0],
            [0, -z, 0],
            [0, 0, -z],
            [y, y, y],
            [-y, -y, -y]
        ]

        self.print_header("TORQUE TESTS: ")

        self.ws("adcs_cmd.rwa_mode", Enums.rwa_modes["RWA_ACCEL_CTRL"])

        for cmd_array in torque_tests:

            self.print_rs("gomspace.vbatt")
            self.print_rs("adcs_monitor.rwa_speed_rd")
            self.logger.put("ADCS_CMD TARGET TORQUE: ")
            self.print_ws("adcs_cmd.rwa_torque_cmd", cmd_array)
            time.sleep(1)
            reading = self.print_rs("adcs_monitor.rwa_torque_rd")
            self.print_rs("adcs_monitor.rwa_speed_rd")
            self.assert_vec_within(abs_of(cmd_array), abs_of(reading), .1)
            self.ws("adcs_cmd.rwa_torque_cmd", [0,0,0])
            self.logger.put("")
            time.sleep(1)

        # Shut down procedure
        self.ws("adcs_cmd.rwa_torque_cmd", [0,0,0])
        time.sleep(1)
        self.ws("adcs_cmd.rwa_speed_cmd", [0,0,0])
        self.ws("adcs_cmd.rwa_mode", Enums.rwa_modes["RWA_SPEED_CTRL"])
        time.sleep(1)

        self.ws("cycle.auto", False)

        self.print_header("WHEEL CHECKOUT COMPLETE")

    def ssa_checkout(self):
        '''
        Prints out all the voltages of each sun sensor, check that they change over time
        '''
        list_of_voltages = []
        for i in range(10):
            voltages = []
            for i in range(0, 20):
                reading = self.print_rs(f"adcs_monitor.ssa_voltage{i}")
                voltages += [reading]
            list_of_voltages += [voltages]
        # check readings changed over time
        self.soft_assert(sum_of_differentials(list_of_voltages) > 0,
            "SSA voltage readings did not vary across readings.") 

    def run(self):
        
        self.print_rs("adcs_monitor.functional")

        # havt_checkout() can still run when adcs_monitor not functional, expect havt_table to be full of 0's
        self.havt_checkout()

        if not self.rs("adcs_monitor.functional"):
            self.logger.put("ADCSC NOT FUNCTIONAL. FINISHING HERE FOR CI - HOOTL CASE")
            self.finish()
            return

        # BEGIN SENSOR CHECKOUT
        self.print_header("Begin Sensor Checkout")
        # expect each sensor value to change from cycle to cycle, given user is jostling the test bed.

        # If either mag1 or mag2 are up, run a check on it.
        if self.rs("adcs_monitor.havt_device1"):
            self.mag_checkout(1)
        if self.rs("adcs_monitor.havt_device2"):
            self.mag_checkout(2)

        # Check Mag Independence
        if self.rs("adcs_monitor.havt_device1") and self.rs("adcs_monitor.havt_device2"):        
            self.mag_independence_checkout()

        # Run checks on GYR if GYR is up.
        if self.rs("adcs_monitor.havt_device0"):
            self.gyr_checkout()
        
        # Run wheel checks if the RWAPOT is up.
        rwa_pot_num = Enums.havt_devices["RWA_POT"]
        if self.rs(f"adcs_monitor.havt_device{rwa_pot_num}"):
            self.wheel_checkout()

        self.ssa_checkout()

        self.print_rs("gomspace.vbatt")
        # TODO FURTHER CHECKOUTS

        self.print_header("ADCS CHECKOUT COMPLETE")
        self.finish()