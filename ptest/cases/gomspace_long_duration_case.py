from .base import SingleSatOnlyCase, TestCaseFailure
from .utils import Enums

class GomspaceLongDurationCheckoutCase(SingleSatOnlyCase):
    def setup_post_bootsetup(self):
        self.print_header("Begin Gomspace Long Duration Checkout Case")
        self.docking_spin_motor_setup()
        self.prop_valves_setup()
        self.adcs_spin_wheels_setup()
        self.cycle()

    def adcs_spin_wheels_setup(self):
        self.logger.put("Begin ADCS Motor Setup")
        self.ws("dcdc.ADCSMotor_cmd", True)
        self.cycle()
        self.ws("adcs.state", Enums.adcs_states["point_manual"])
        self.ws("adcs_cmd.rwa_mode", Enums.rwa_modes["RWA_SPEED_CTRL"])
        self.ws("adcs_cmd.rwa_speed_cmd", [0,0,0])

    def prop_valves_setup(self):
        self.logger.put("Begin Valve Setup")
        self.ws("dcdc.SpikeDock_cmd", True) 
        self.flight_controller.write_state("prop.state", Enums.prop_states["manual"])

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

    def turn_motors_on(self):
        if self.rs("docksys.dock_config") == "true" and self.rs("docksys.config_cmd" == "true"):
            self.ws("docksys.config_cmd", "false")
        elif self.rs("docksys.dock_config") == "false" and self.rs("docksys.config_cmd" == "false"):
            self.ws("docksts.config_cmd", "true")
    
    def turn_motors_off(self):
        if self.rs("docksys.dock_config") == "false" and self.rs("docksys.config_cmd" == "true"):
            self.ws("docksys.config_cmd", "true")
        elif self.rs("docksys.dock_config") == "true" and self.rs("docksys.config_cmd" == "false"):
            self.ws("docksts.config_cmd", "false")

    def run_case_singlesat(self):
        self.cycle_no = self.rs("pan.cycle_no")
        self.fire_valves()
        edu_vbatt_threshold = 7000
        num_test_cycles = 6000

        for _ in range(num_test_cycles):

            #check if vbatt is within desired range (terminate if not), log data every 10 seconds
            self.cycle_no = self.rs("pan.cycle_no")
            vbatt = int(self.rs("gomspace.vbatt"))
            if vbatt < edu_vbatt_threshold:
                self.logger.put("Vbatt is under threshold at: " + str(vbatt) + "mV " + "during cycle: " + str(self.cycle_no))
                self.logger.put("Terminating test.")
                break
            if self.cycle_no % 100 == 0: 
                self.logger.put("Vbatt: " + str(vbatt) + " mV")

            #perform either a wheel, docking, or valve commands every minute 
            cycle_quantum  = 600*3 
            if self.cycle_no % cycle_quantum in range(0, cycle_quantum//3):
                self.logger.put("Spinning ADCS wheels at speed 100.")
                self.spin_motors(100)
            elif self.cycle_no % cycle_quantum in range(cycle_quantum//3, cycle_quantum*2//3):
                self.logger.put("Stopping ADCS wheels, turning docking motor.")
                self.spin_motors(0)
                self.turn_motors_on()
            else:
                self.logger.put("Stopping docking motor, firing valves.")
                self.fire_valves() 
                self.turn_motors_off()

            self.cycle()

        self.finish()
