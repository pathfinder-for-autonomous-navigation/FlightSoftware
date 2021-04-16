from .base import SingleSatCase, PSimCase
from .utils import FSWEnum, Enums, TestCaseFailure
from psim.sims import SingleAttitudeOrbitGnc

class PiksiFaultHandler(SingleSatCase, PSimCase):
    def __init__(self, *args, **kwargs):
        super(PiksiFaultHandler, self).__init__(*args, **kwargs)

        self.initial_state = "standby"
        self.psim_configs += ["truth/standby"]

    def post_boot(self):
        self.mission_state = "leader"
        self.cycle()

    def collect_diagnostic_data(self):
        self.rs("piksi.state")
        self.rs("pan.state")
        self.rs("pan.cycle_no")

    def run(self):
        self.collect_diagnostic_data()

        # TODO implement testcase here.

        self.finish()
