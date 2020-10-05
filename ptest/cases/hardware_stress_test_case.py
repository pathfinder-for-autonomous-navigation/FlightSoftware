from .base import SingleSatOnlyCase, TestCaseFailure
from .utils import Enums

class HardwareStressCheckoutCase(SingleSatOnlyCase):
    def setup_post_bootsetup(self):
        self.print_header("Begin Hardware Stress Test Checkout Case")
        self.docking_spin_motor_setup()
        self.prop_valves_setup()
        self.adcs_spin_wheels_setup()
        self.cycle()

    def adcs_spin_wheels_setup(self):
        self.print_header("Begin Motor Setup")
        self.ws("dcdc.ADCSMotor_cmd", True)
        self.cycle()
        self.ws("adcs.state", Enums.adcs_states["point_manual"])
        self.ws("adcs_cmd.rwa_mode", Enums.rwa_modes["RWA_SPEED_CTRL"])
        self.ws("adcs_cmd.rwa_speed_cmd", [0,0,0])

    def prop_valves_setup(self):
        self.ws("dcdc.SpikeDock_cmd", True) 
        self.sim.flight_controller.write_state(
            "prop.state", Enums.prop_states["disabled"])

    def docking_spin_motor_setup(self):
        self.write_state("dcdc.SpikeDock_cmd", "true") #should this be "true" or True ?

    def spin_motors(self, speed):
        assert(speed in range(-680, 681))
        self.ws("adcs_cmd.rwa_speed_cmd", [speed, speed, speed])

    def fire_valves(self):
        #TODO set schedule
        #TODO cycle, repeat
        pass

    def turn_motors(self):
        if self.rs("docksys.dock_config") == "true" and self.rs("docksys.config_cmd" == "true"):
            self.ws("docksys.config_cmd", "false")
        elif self.rs("docksys.dock_config") == "false" and self.rs("docksys.config_cmd" == "false"):
            self.ws("docksts.config_cmd", "true")

    def run_case_singlesat(self):
        self.cycle_no = self.rs("pan.cycle_no")
        self.spin_motors(680)
        self.fire_valves()
        for _ in range(600):
            self.turn_motors()
            self.cycle()


        self.finish()