# Runs mission from startup state to standby state.
from .base import SingleSatCase
from psim.sims import SingleAttitudeOrbitGnc
from .utils import Enums, TestCaseFailure

class DeploymentToStandby(SingleSatCase):
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
        return "standby"

    def setup_post_bootsetup(self):
        self.sim.mock_sensor_validity = True
        return

    def run_case_singlesat(self):
        if self.rs("pan.cycle_no") > 300:
            raise TestCaseFailure("Nominal detumbling for ci should take < 300 cycles")
        self.finish()
