# Runs mission from startup state to standby state.
from .base import SingleSatOnlyCase

class DeploymentToStandby(SingleSatOnlyCase):
    @property
    def sim_duration(self):
        return float("inf")

    @property
    def desired_initial_state(self):
        return "standby"

    @property
    def fast_boot(self):
        return False

    @property
    def sim_initial_state(self):
        # Start the spacecraft with zero angular rate in the body frame, 
        # so that we exit detumble almost immediately.
        return "startup"

    def run_case_singlesat(self):
        self.finish()
