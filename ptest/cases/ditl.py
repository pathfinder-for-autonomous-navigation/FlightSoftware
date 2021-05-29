# Runs mission from startup state to standby state.
from psim.sims import SingleAttitudeOrbitGnc
from .utils import Enums, mag_of, sum_of_differentials
from .mission import SingleSatDetumbleCase

class Ditl(SingleSatDetumbleCase):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def data_logs(self):
        self.rs("attitude_estimator.valid")
        self.rs("pan.deployment.elapsed")
        self.rs("pan.state")
        self.rs("radio.state")
        self.rs("pan.cycle_no")
        self.rs("adcs.state")
        self.rs("attitude_estimator.valid")
        self.rs("time.valid")
        self.rs("orbit.valid")
        
        self.rs("sys.memory_use")


    def post_boot(self):
        self.ws("fault_handler.enabled", True)
        

    def run(self):
        self.cycle()
        self.data_logs()