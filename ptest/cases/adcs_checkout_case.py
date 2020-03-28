# ADCSCheckoutCase. Verifies the functionality of the ADCS.
from .base import SingleSatOnlyCase

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

        # cache the initially functional devices
        initial_up_devices = self.havt_read

        # disable all devices
        for x in range(self.havt_length):
            self.ws(f"adcs_cmd.havt_disable{x}", True)

        self.step()

        print("Post disabling all devices:")
        self.print_havt_read()
        assert([0 for x in range(self.havt_length)] == self.havt_read), "Disabling all devices failed"

        # reset all devices
        for x in range(self.havt_length):
            self.ws(f"adcs_cmd.havt_reset{x}", True)
        self.step()
        print("Post resetting all devices:")
        self.print_havt_read()
        assert(initial_up_devices == self.havt_read), "Disable Reset Cycle Failed, New HAVT Table does not match initial table cache"

        print("Reset-Disable Success. All initially functional devices remain functional.")

        self.print_header("ADCS Checkout Case Complete")