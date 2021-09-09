from .base import DualSatCase
from .utils import Enums, TestCaseFailure
import time

class PropFiringCase(DualSatCase):

    def __init__(self, *args, **kwargs): #allows for interaction with the terminal during ptesting
        super().__init__(*args, **kwargs)
        self.debug_to_console = True

    def run(self):
        self.ws("prop.threshold_firing_pressure", 10)
        self.ws("prop.cycles_until_firing", 1500) #make the cycles until firing be 2000 to allow time for calculations before firing
        self.ws("prop.tank2.pressure", 26)
        self.mission_state = "auto"
        self.ws("cycle.auto", True)
        propNum = self.rs("prop.state")
        while (propNum < 6):
            self.print_rs("prop.state")
            propNum = self.rs("prop.state")
            self.print_rs("orbit.control.valve1")
            self.print_rs("prop.cycles_until_firing")
            #self.print_rs("prop.tank2.pressure")
        self.ws("prop.state", 6) #get prop state into await firing mode
        while (self.rs("prop.cycles_until_firing") > 0):
            self.print_rs("orbit.control.J_ecef")
            self.print_rs("prop.state")
            self.print_rs("orbit.control.valve1")
            self.print_rs("prop.cycles_until_firing")


