from .utils import Enums, TestCaseFailure
import time

from .mission import (
    DualSatNearFieldCase
)

class PropFiringCase(DualSatNearFieldCase):

    def __init__(self, *args, **kwargs): #allows for interaction with the terminal during ptesting
        super().__init__(*args, **kwargs)
        self.debug_to_console = True

    def run(self):
        self.follower.ws("prop.threshold_firing_pressure", 10)
        propNum = self.rs("prop.state")
        self.print_rs("orbit.control.valve1")
        while(propNum != 7):
            self.print_rs("prop.state")
            propNum = self.rs("prop.state")
            self.print_rs("orbit.control.valve1")
            self.print_rs("orbit.valid")
            self.print_rs("rel_orbit.state")
            self.print_rs("prop.cycles_until_firing")
            #self.ws("prop.state", 6) #get prop state into await firing mode
            self.print_rs("orbit.control.J_ecef")
            self.print_rs("prop.state")
            self.print_rs("orbit.control.valve1")
            self.print_rs("prop.cycles_until_firing")
            self.print_rs("pan.state")
            self.cycle()
        


