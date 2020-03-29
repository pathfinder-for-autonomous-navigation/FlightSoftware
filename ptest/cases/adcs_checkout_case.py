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

    def rs(self, name):
        assert(isinstance(name, str)), "State field name was not a string."
        ret = self.sim.flight_controller.smart_read(name)
        return ret

    def print_rs(self, name):
        print(f"{name} is {self.rs(name)}")
    
    def ws(self, name, val):
        self.sim.flight_controller.write_state(name, val)
        read_return = self.rs(name)
        assert(read_return == val), f"Write state not applied, expected: {val}, got {read_return} instead"

    def print_header(self, title):
        print()
        print(title)
        print()

    def soft_assert(self, condition, *args):
        '''
        Soft assert prints a fail message if the condition is False
        
        If specificied with a fail message, then a pass message, 
        it will also print a pass message if condition is True.
        '''
        if condition: 
            if len(args) == 1:
                pass
            else:
                print(args[1])
        else: 
            print()
            print(f"$ SOFT ASSERTION ERROR: {args[0]}")
            print()

    # @property
    # def mission_mode(self):
    #     return self.sim.flight_controller.smart_read("pan.state")


    # @mission_mode.setter
    # def mission_mode(self, state):
    #     print(f"Mission mode set to: {state}")
    #     assert isinstance(state, int), f"Expected int, got {state} instead."
    #     self.sim.flight_controller.write_state("pan.state", state)

    # @property
    # def adcs_func(self):
    #     return self.sim.flight_controller.smart_read("adcs_monitor.functional")
    
    # @property
    # def cycle_no(self):
    #     return self.sim.flight_controller.smart_read("pan.cycle_no")

    @property
    def havt_read(self):
        read_list = [False for x in range(self.havt_length)]
        for x in range(self.havt_length):
            read_list[x] = self.rs("adcs_monitor.havt_device"+str(x))
        return read_list

    # @property
    # def rwa_speed_cmd(self):
    #     print("RWA SPEED CMD GETTER CALLED\n")
    #     return self.sim.flight_controller.smart_read("adcs_cmd.rwa_speed_cmd")
    
    # @property
    # def rwa_torque_cmd(self):
    #     return self.sim.flight_controller.smart_read("adcs_cmd.rwa_torque_cmd")

    # @property
    # def rwa_mode_cmd(self):
    #     print("MODE GETTER\n")
    #     return self.sim.flight_controller.smart_read("adcs_cmd.rwa_mode")

    # @rwa_speed_cmd.setter
    # def rwa_speed_cmd(self, rwa_list):
    #     assert( len(rwa_list) == 3)
    #     self.sim.flight_controller.write_state("adcs_cmd.rwa_speed_cmd", *rwa_list)

    # @rwa_torque_cmd.setter
    # def rwa_torque_cmd(self, rwa_list):
    #     assert( len(rwa_list) == 3)        
    #     self.sim.flight_controller.write_state("adcs_cmd.rwa_torque_cmd", *rwa_list)

    # @rwa_mode_cmd.setter
    # def rwa_mode_cmd(self, val):
    #     self.sim.flight_controller.write_state("adcs_cmd.rwa_mode", val)

    def setup_case_singlesat(self):
        self.havt_length = 18 # _LENGTH in havt_devices.hpp

        # copied from havt_devices.hpp
        self.havt_devices = ["IMU_GYR",
        "IMU_MAG1",
        "IMU_MAG2",
        "MTR1",
        "MTR2",
        "MTR3",
        "RWA_POT",
        "RWA_WHEEL1",
        "RWA_WHEEL2",
        "RWA_WHEEL3",
        "RWA_ADC1",
        "RWA_ADC2",
        "RWA_ADC3",
        "SSA_ADC1",
        "SSA_ADC2",
        "SSA_ADC3",
        "SSA_ADC4",
        "SSA_ADC5"]

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
                print(f"Device #{x}, {self.havt_devices[x]} is not functional")

        self.print_header("Finished Initialization")

        print("Initial HAVT Table:")
        self.print_havt_read()

        # Note IMUGYR on left
        # Just FC and ADCSC
        barebones_hitl = "000111011100" + "000000"

        # FC + ADCSC + MAG1 + MAG2 + GYR
        minimal_hitl   = "111111011100" + "000000"
            
        # I forgot what it actually is, will update on next PR
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

        # TODO make section compatible with 2x imu active
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