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
        self.logger.put("Begin ADCS Motor Setup")
        self.ws("dcdc.ADCSMotor_cmd", True)
        self.cycle()
        self.ws("adcs.state", Enums.adcs_states["manual"])
        self.ws("adcs_cmd.rwa_mode", Enums.rwa_modes["RWA_SPEED_CTRL"])
        self.ws("adcs_cmd.rwa_speed_cmd", [0,0,0])

    def prop_valves_setup(self):
        self.logger.put("Begin Valve Setup")
        self.ws("dcdc.SpikeDock_cmd", True) 
        self.sim.flight_controller.write_state(
            "prop.state", Enums.prop_states["disabled"])

    def docking_spin_motor_setup(self):
        self.logger.put("Begin Docking Motor Setup")
        self.ws("dcdc.SpikeDock_cmd", "true") #should this be "true" or True ?

    def spin_motors(self, speed):
        assert(speed in range(-680, 681))
        self.ws("adcs_cmd.rwa_speed_cmd", [speed, speed, speed])

    def fire_valves(self):
        val = "0"
        if self.rs("pan.cycle_no")%2 == 0:
            val = "1"
        self.ws("prop.sched_valve1", val)
        self.ws("prop.sched_valve2", val)
        self.ws("prop.sched_valve3", val)
        self.ws("prop.sched_valve4", val)

    def turn_motors(self):
        if self.rs("docksys.dock_config") == "true" and self.rs("docksys.config_cmd" == "true"):
            self.ws("docksys.config_cmd", "false")
        elif self.rs("docksys.dock_config") == "false" and self.rs("docksys.config_cmd" == "false"):
            self.ws("docksts.config_cmd", "true")

    def run_case_singlesat(self):
        self.cycle_no = self.rs("pan.cycle_no")
        self.logger.put("Turning on wheels at speed 680.")
        self.spin_motors(680)
        self.fire_valves()
        self.logger.put("Turning on the docking motor and begin firing valves.")
        for _ in range(600):
            self.turn_motors()
            self.fire_valves() 
            self.cycle()
            self.cycle_no = self.rs("pan.cycle_no")
            if self.cycle_no % 20 == 0:
                self.logger.put("Current cycle: " + str(self.cycle_no))

        self.finish()