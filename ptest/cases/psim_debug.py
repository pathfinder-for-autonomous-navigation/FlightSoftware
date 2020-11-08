# Runs mission from startup state to standby state.
from .base import SingleSatOnlyCase
from psim.sims import SingleAttitudeOrbitGnc

class PsimDebug(SingleSatOnlyCase):
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
        return "detumble"

    @property
    def fast_boot(self):
        return False

    @property
    def sim_initial_state(self):
        # Start the spacecraft with zero angular rate in the body frame, 
        # so that we exit detumble almost immediately.
        return "startup"

    def setup_post_bootsetup(self):
        self.print_ws("detumble_safety_factor",0.4)

    def run_case_singlesat(self):
        # self.print_rs_psim("truth.leader.environment.s.body")

        self.print_rs("pan.state")
        self.print_rs("adcs.state")
        self.print_rs("pan.cycle_no")
        self.print_rs("orbit.time")
        self.print_rs("orbit.pos")
        self.print_rs("orbit.vel")
        self.print_rs('adcs_monitor.ssa_vec')
        self.print_rs('adcs_monitor.mag1_vec')
        self.print_rs('adcs_monitor.gyr_vec')
        self.print_rs("attitude_estimator.w_body")
        self.print_rs("adcs_cmd.mtr_cmd")
        self.print_rs("adcs_cmd.rwa_torque_cmd")

        pass