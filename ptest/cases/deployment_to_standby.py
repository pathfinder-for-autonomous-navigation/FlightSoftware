# Runs mission from startup state to standby state.
from .base import SingleSatOnlyCase
from psim.sims import SingleAttitudeOrbitGnc

class DeploymentToStandby(SingleSatOnlyCase):
    @property
    def sim_configs(self):
        configs = ["truth/deployment", "truth/base"]
        configs += ["sensors/base"]
        return configs

    @property
    def sim_model(self):
        return SingleAttitudeOrbitGnc

    @property
    def sim_duration(self):
        return float("inf")

    @property
    def initial_state(self):
        return "standby"

    @property
    def fast_boot(self):
        return False

    @property
    def sim_initial_state(self):
        # Start the spacecraft with zero angular rate in the body frame, 
        # so that we exit detumble almost immediately.
        return "startup"

    def setup_post_bootsetup(self):
        return
        # self.print_ws("detumble_safety_factor",0.0)

    def run_case_singlesat(self):
        self.finish()
