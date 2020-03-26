# ADCSCheckoutCase. Verifies the functionality of the ADCS.
from .base import SingleSatOnlyCase

class ADCSCheckoutCase(SingleSatOnlyCase):
    def setup_case_singlesat(self):
        self.cycle_no = -1
        self.adcs_func = False

        self.havt_length = 18 # _LENGTH in havt_devices.hpp
        self.havt_device = [False for x in range(self.havt_length)]

        self.sim.flight_controller.write_state("pan.state", 11) # Mission State = Manual
        self.sim.flight_controller.write_state("adcs.state", 5) # ADCS State = Manual
        self.sim.flight_controller.write_state("adcs_cmd.rwa_mode", 1) # Speed Control
        self.sim.flight_controller.write_state("adcs_cmd.rwa_speed_cmd", "0, 0, 0") # 0 speed to begin with
        self.sim.flight_controller.write_state("dcdc.ADCSMotor_cmd", "true")

    def step(self):
        self.sim.flight_controller.write_state("cycle.start", "true")

    def update_vars(self):
        self.cycle_no = self.sim.flight_controller.read_state("pan.cycle_no")
        self.adcs_func = self.sim.flight_controller.read_state("adcs_monitor.functional")

        for x in range(self.havt_length):
            self.havt_device[x] = self.sim.flight_controller.read_state("adcs_monitor.havt_device"+str(x))

    def run_case_singlesat(self):
        self.update_vars()
        print("Cycle Number: "+str(self.cycle_no))
        self.step()
        self.update_vars()
        print("Cycle Number: "+str(self.cycle_no))
