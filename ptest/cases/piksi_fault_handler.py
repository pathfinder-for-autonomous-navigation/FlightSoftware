from .base import SingleSatOnlyCase
from .utils import FSWEnum, Enums, TestCaseFailure
from psim.sims import SingleAttitudeOrbitGnc

class PiksiFaultHandler(SingleSatOnlyCase):
    @property
    def sim_configs(self):
        configs = ["truth/ci", "truth/base"]
        configs += ["sensors/base"]
        return configs

    @property
    def sim_model(self):
        return SingleAttitudeOrbitGnc

    @property
    def sim_mapping(self):
        return "ci_mapping.json"

    @property
    def sim_duration(self):
        return float("inf")

    @property
    def initial_state(self):
        return "leader"

    @property
    def fast_boot(self):
        return True

    def collect_diagnostic_data(self):
        self.rs("piksi.state")
        self.rs("pan.state")
        self.rs("pan.cycle_no")

    def run_case_singlesat(self):
        self.collect_diagnostic_data()

        # TODO implement testcase here.

        self.finish()
