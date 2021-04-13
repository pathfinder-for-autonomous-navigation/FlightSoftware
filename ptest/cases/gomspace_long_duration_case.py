from .base import SingleSatCase
from .utils import Enums
from .hardware_stress_test_case import ActuateHardwareCase

class GomspaceLongDurationCheckoutCase(ActuateHardwareCase):
    def post_boot(self):
        self.print_header("Begin Gomspace Long Duration Checkout Case")
        self.setup_hardware()
        self.cycle()

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
            cycle_quantum  = 20*3 
            if self.cycle_no % cycle_quantum in range(0, cycle_quantum//3):
                self.logger.put("Spinning ADCS wheels at speed 100.")
                self.spin_motors(100)
            elif self.cycle_no % cycle_quantum in range(cycle_quantum//3, cycle_quantum*2//3):
                self.logger.put("Stopping ADCS wheels, turning docking motor.")
                self.spin_motors(0)
                self.turn_motor_on()
            else:
                self.logger.put("Stopping docking motor, firing valves.")
                self.fire_valves() 
                self.turn_motor_off()

            self.cycle()

        self.finish()
