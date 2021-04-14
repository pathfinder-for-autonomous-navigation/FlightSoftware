from .base import SingleSatCase
from .utils import Enums

class ActuateHardwareCase(SingleSatCase):
    def post_boot(self):
        self.mission_state = "manual"
        self.cycle()

    def setup_hardware(self):
        self.docking_spin_motor_setup()
        self.prop_valves_setup()
        self.adcs_spin_wheels_setup()

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
        self.flight_controller.write_state(
            "prop.state", Enums.prop_states["manual"])

    def docking_spin_motor_setup(self):
        self.logger.put("Begin Docking Motor Setup")
        self.ws("dcdc.SpikeDock_cmd", True)

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

    def turn_motor(self):
        if self.rs("docksys.dock_config") == True and self.rs("docksys.config_cmd") == True:
            self.ws("docksys.config_cmd", False)
        elif self.rs("docksys.dock_config") == False and self.rs("docksys.config_cmd") == False:
            self.ws("docksys.config_cmd", True)
    
    def turn_motor_on(self):
        if self.rs("docksys.dock_config") == True and self.rs("docksys.config_cmd") == True:
            self.ws("docksys.config_cmd", False)
        elif self.rs("docksys.dock_config") == False and self.rs("docksys.config_cmd") == False:
            self.ws("docksys.config_cmd", True)
    
    def turn_motor_off(self):
        if self.rs("docksys.dock_config") == False and self.rs("docksys.config_cmd") == True:
            self.ws("docksys.config_cmd", False)
        elif self.rs("docksys.dock_config") == True and self.rs("docksys.config_cmd") == False:
            self.ws("docksys.config_cmd", True)

class HardwareStressCheckoutCase(ActuateHardwareCase):
    def post_boot(self):
        super(HardwareStressCheckoutCase, self).post_boot()

        self.print_header("Begin Hardware Stress Test Checkout Case")
        self.setup_hardware()
        self.cycle()

    def run(self):
        self.cycle_no = self.rs("pan.cycle_no")
        self.logger.put("Turning on wheels at speed 680.")
        self.spin_motors(680)
        self.fire_valves()
        self.logger.put("Turning on the docking motor.")
        for _ in range(300):
            self.turn_motor()
            self.cycle()
            self.cycle_no = self.rs("pan.cycle_no")
            if self.cycle_no % 20 == 0:
                self.logger.put("Current cycle: " + str(self.cycle_no))
        
        self.logger.put("Turning off the docking motor and beginning firing valves.")
        self.turn_motor_off()
        for _ in range(300):
            self.fire_valves()
            self.cycle()
            self.cycle_no = self.rs("pan.cycle_no")
            if self.cycle_no % 20 == 0:
                self.logger.put("Current cycle: " + str(self.cycle_no))

        self.finish()
