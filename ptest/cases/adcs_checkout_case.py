# ADCSCheckoutCase. Verifies the functionality of the ADCS.
from .base import SingleSatOnlyCase

class ADCSCheckoutCase(SingleSatOnlyCase):

    @property
    def adcs_func(self):
        return self.sim.flight_controller.read_state("adcs_monitor.functional")
    
    @property
    def cycle_no(self):
        return self.sim.flight_controller.read_state("pan.cycle_no")

    @property
    def havt_read(self):
        self.havt_length = 18 # _LENGTH in havt_devices.hpp
        read_list = [False for x in range(self.havt_length)]
        for x in range(self.havt_length):
            read_list = self.sim.flight_controller.read_state("adcs_monitor.havt_device"+str(x))
        read_list = [True if x is true else False for x in read_list]
        return read_list

    def setup_case_singlesat(self):
        self.havt_length = 18 # _LENGTH in havt_devices.hpp
        self.havt_device = [False for x in range(self.havt_length)]

        self.sim.flight_controller.write_state("pan.state", 11) # Mission State = Manual
        self.sim.flight_controller.write_state("adcs.state", 5) # ADCS State = Manual
        self.sim.flight_controller.write_state("adcs_cmd.rwa_mode", 1) # Speed Control
        self.sim.flight_controller.write_state("adcs_cmd.rwa_speed_cmd", "0, 0, 0") # 0 speed to begin with
        self.sim.flight_controller.write_state("dcdc.ADCSMotor_cmd", "true")

    """Steps the FC forward by one step"""
    def step(self):
        self.sim.flight_controller.write_state("cycle.start", "true")

    def run_case_singlesat(self):
        print("Cycle Number: "+str(self.cycle_no))
        print("ADCS Functional: "+str(type(self.adcs_func)))
        self.step()
        print("Cycle Number: "+str(self.cycle_no))
