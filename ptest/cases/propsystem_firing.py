from .base import SingleSatCase
from .utils import Enums, TestCaseFailure
import time

class PropFiringCase(SingleSatCase):

    def __init__(self, *args, **kwargs): #allows for interaction with the terminal during ptesting
        super().__init__(*args, **kwargs)

    def run(self):
        self.mission_state = "auto"
        self.ws("cycle.auto", True)
        propNum = self.rs("prop.state")
        while (propNum < 6):
            self.print_header(self.rs("prop.state"))
            propNum = self.rs("prop.state")
        self.print_header(self.rs("prop.state"))
        self.ws("prop.state", 6) #get prop state into await firing mode
        while (self.rs("prop.cycles_until_firing") > 0):
            self.print_header(self.rs("prop.cycles_until_firing"))
        self.ws("prop.cycles_until_firing", 20) #make the cycles until firing be 20 to allow time for calculations before firing


