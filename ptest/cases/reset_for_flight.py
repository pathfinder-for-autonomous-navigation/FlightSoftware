from .base import SingleSatCase
from .utils import Enums, TestCaseFailure
import math

class ResetforFlight(SingleSatCase):
    def run(self):
        self.mission_state = "manual"
        self.ws( "cycle.auto", False )
        self.cycle()

        self.print_header("Starting Reset")

        #printing original states
        self.print_header("initial pan.bootcount: \n" 
                        + str(self.rs("pan.bootcount")))
        self.print_header("initial pan.deployed: \n" 
                + str(self.rs("pan.deployed")))
        self.print_header("initial pan.deployment.elapsed: \n" 
                        + str(self.rs("pan.deployment.elapsed")))
        self.print_header("initial pan.kill_switch: \n" 
                + str(self.rs("pan.kill_switch")))

        cycle_no = self.rs("pan.cycle_no")
        #pan.bootcount & pan.deployed have longest save duration of 1000 cycles 
        #additional 10 cycles to make sure fields are saved to EEPROM
        cycle_duration = cycle_no + 1000 + 10 

        while (cycle_no < cycle_duration):
            self.print_header("cycle_no: " + str(self.rs("pan.cycle_no")))
            self.ws("pan.bootcount", 0)
            self.ws("pan.deployed", False)
            self.ws("pan.deployment.elapsed", 0)
            self.ws("pan.kill_switch", 0)
            self.ws("pan.attitude_estimator.ignore_sun_vectors", False)
            self.cycle()

            cycle_no = self.rs("pan.cycle_no")

        #printing updated states after cycling
        self.print_header("final pan.bootcount: \n" 
                            + str(self.rs("pan.bootcount")))
        self.print_header("final pan.deployed: \n" 
                    + str(self.rs("pan.deployed")))
        self.print_header("final pan.deployment.elapsed: \n" 
                            + str(self.rs("pan.deployment.elapsed")))
        self.print_header("final pan.kill_switch: \n" 
                    + str(self.rs("pan.kill_switch")))


        self.print_header("Flight Reset Complete")
        self.finish()
