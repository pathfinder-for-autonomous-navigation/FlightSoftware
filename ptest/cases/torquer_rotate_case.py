from .base import SingleSatOnlyCase, TestCaseFailure
from .utils import Enums

class MTorquerCase(SingleSatOnlyCase):

    def read_state(self, string_state):
        return self.sim.flight_controller.read_state(string_state)
    
    def write_state(self, string_state, state_value):
        self.sim.flight_controller.write_state(string_state, state_value)
        return self.read_state(string_state)

    def log_states(self):
        self.logger.put("<add every state that needs logging here>")

    def run_case_singlesat(self):
        self.print_rs("adcs_monitor.functional")
        
#turn the magnetorquers and log all the fields 
#sim.flightcontroller
    