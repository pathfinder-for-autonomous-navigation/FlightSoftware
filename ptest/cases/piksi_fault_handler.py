from .base import SingleSatOnlyCase
from .utils import FSWEnum, Enums, BootUtil

class PiksiFaultHandler(SingleSatOnlyCase):
    @property
    def sim_duration(self):
        return float("inf")

    @property
    def initial_state(self):
        return "leader"

    @property
    def fast_boot(self):
        return False

    def collect_diagnostic_data(self):
        self.rs("piksi.state")
        self.rs("pan.state")
        self.rs("pan.cycle_no")

    def run_case_singlesat(self):
        self.collect_diagnostic_data()

        # TODO implement testcase here.

        self.finish()
