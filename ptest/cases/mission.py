# Runs mission from startup state to standby state.
from .base import SingleSatOnlyCase

class DeploymentToStandby(SingleSatOnlyCase):
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

    def run_case_singlesat(self):

        self.print_rs("pan.state")
        self.print_rs("adcs.state")
        self.print_rs("adcs_cmd.mtr_cmd")
        self.print_rs("adcs_cmd.rwa_torque_cmd")
        self.print_rs("attitude.pointer_vec1_desired")
        self.print_rs("attitude.pointer_vec2_desired")
        self.print_rs("attitude.pointer_vec1_current")
        self.print_rs("attitude.pointer_vec2_current")