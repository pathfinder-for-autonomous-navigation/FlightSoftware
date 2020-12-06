# Runs mission from startup state to standby state.
from .base import SingleSatOnlyCase
from psim.sims import SingleAttitudeOrbitGnc

class PsimDebug(SingleSatOnlyCase):
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
    def debug_to_console(self):
        return True

    @property
    def sim_duration(self):
        return float("inf")

    @property
    def initial_state(self):
        return "startup"

    @property
    def fast_boot(self):
        return False

    @property
    def sim_initial_state(self):
        return "startup"

    def setup_post_bootsetup(self):
        return

    def data_logs(self):

        self.rs("pan.deployment.elapsed")
        self.rs("pan.state")
        self.rs("pan.cycle_no")
        self.rs("pan.bootcount")
        self.rs("adcs.state")
        self.rs("orbit.time")
        self.rs("orbit.pos")
        self.rs("orbit.vel")
        self.rs('adcs_monitor.ssa_vec')
        self.rs('adcs_monitor.mag1_vec')
        self.rs('adcs_monitor.gyr_vec')
        self.rs("attitude_estimator.q_body_eci")
        self.rs("attitude_estimator.w_body")
        self.rs("attitude_estimator.fro_P")
        self.rs("adcs_cmd.mtr_cmd")
        self.rs("adcs_cmd.rwa_torque_cmd")

    def run_case_singlesat(self):
        # self.print_rs_psim("truth.leader.environment.s.body")
        self.rs_psim("truth.leader.attitude.w")
        self.data_logs()
