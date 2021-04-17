# Runs mission from startup state to standby state.
from .base import SingleSatOnlyCase
from psim.sims import SingleAttitudeOrbitGnc
from .utils import Enums, mag_of, sum_of_differentials

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
        return True

    @property
    def sim_initial_state(self):
        return "startup"

    @property
    def sim_ic_map(self):
        ret = {}
        ret["truth.t.ns"] = 420000000*10
        return ret

    def setup_post_bootsetup(self):
        # self.print_ws("pan.state", Enums.mission_states['standby'])
        # self.print_ws("adcs.state", Enums.adcs_states['point_standby'])
        self.sim.mock_sensor_validity = True
        
        return

    def data_logs(self):

        self.rs("pan.deployment.elapsed")
        self.rs("pan.state")
        self.rs("radio.state")
        self.rs("pan.cycle_no")
        #self.rs("pan.bootcount")
        self.rs("adcs.state")
        #self.rs("piksi.time")
        #self.rs("orbit.pos")
        #self.rs("orbit.vel")
        #self.rs('adcs_monitor.ssa_vec')
        #self.rs('adcs_monitor.mag1_vec')
        #self.rs('adcs_monitor.gyr_vec')
        
        #self.rs('adcs_monitor.ssa_mode')

        #self.rs("attitude_estimator.q_body_eci")
        #self.rs("attitude_estimator.L_body")

        #self.rs("adcs_cmd.mtr_cmd")
        #self.rs("adcs_cmd.rwa_torque_cmd")
        self.rs("attitude_estimator.valid")
        #self.rs("attitude_estimator.w_bias_body"),
        self.rs("time.valid")
        self.rs("orbit.valid")
        
        self.rs("sys.memory_use")

    def run_case_singlesat(self):
        #self.rs_psim("truth.leader.attitude.w")
        #self.rs_psim("truth.t.ns")
        #self.rs_psim("truth.dt.ns")

        self.data_logs()
