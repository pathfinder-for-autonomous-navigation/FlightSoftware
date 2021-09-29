from .utils import Enums, TestCaseFailure
import time

from .mission import (
    DualSatNearFieldCase
)

class PropFiringCase(DualSatNearFieldCase):

    def __init__(self, *args, **kwargs): #allows for interaction with the terminal during ptesting
        super().__init__(*args, **kwargs)
        self.debug_to_console = True

    def post_boot(self):
        super(PropFiringCase, self).post_boot()
        self.follower.ws("adcs.state", 1)
        self.follower.ws("dcdc.ADCSMotor_cmd", False)

    def run(self):
        i=0
        propNum = self.rs("prop.state")
        self.print_rs("orbit.control.valve1")
        while(propNum != 7):
            self.follower.ws("prop.threshold_firing_pressure", 200)
            self.print_rs("prop.state")
            propNum = self.rs("prop.state")
            self.print_rs("prop.tank2.pressure")
            self.print_rs("prop.threshold_firing_pressure")
            self.print_rs("orbit.control.valve1")
            self.print_rs("prop.cycles_until_firing")
            self.print_rs("orbit.control.J_ecef")
            self.print_rs("prop.state")
            self.print_rs("prop.cycles_until_firing")
            self.print_rs("gomspace.vbatt")
            if (propNum == 3 and i<100):
                i += 1
            elif (propNum == 3 and i==100):
                self.follower.ws("prop.threshold_firing_pressure", 10)
                i = 0
            self.cycle()