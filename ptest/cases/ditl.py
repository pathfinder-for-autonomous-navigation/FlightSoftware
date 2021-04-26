# Runs mission from startup state to standby state.
from psim.sims import SingleAttitudeOrbitGnc
from .utils import Enums, mag_of, sum_of_differentials
from .mission.standby import SingleSatStandbyCase

class Ditl(SingleSatStandbyCase):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def data_logs(self):
        self.rs("attitude_estimator.valid")
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

    # def run_case_singlesat(self):
    #     #self.rs_psim("truth.leader.attitude.w")
    #     #self.rs_psim("truth.t.ns")
    #     #self.rs_psim("truth.dt.ns")

    #     self.data_logs()

    def run(self):
        self.cycle()
        self.data_logs()