# Runs mission from startup state to standby state.
from .base import SingleSatCase
from .base import PSimCase
from psim.sims import SingleAttitudeOrbitGnc
from .utils import Enums, mag_of, sum_of_differentials
import lin

class PSimDebug(SingleSatCase, PSimCase):
    """
    comments
    """
    def __init__(self, *args, **kwargs):
        super(PSimDebug, self).__init__(*args, **kwargs)
        self.initial_state = "startup"
        self.psim_configs += ['truth/deployment']
        self.psim_config_overrides["truth.leader.attitude.w"] = lin.Vector3([0.01,0.0738,-0.01])

    def data_logs(self):
        
        self.rs("pan.deployment.elapsed")
        self.rs("pan.state")
        self.rs("radio.state")
        self.rs("pan.cycle_no")
        self.rs("pan.bootcount")
        self.rs("adcs.state")
        self.rs("piksi.time")
        self.rs("orbit.pos")
        self.rs("orbit.vel")
        self.rs('adcs_monitor.ssa_vec')
        self.rs('adcs_monitor.mag1_vec')
        self.rs('adcs_monitor.gyr_vec')
        
        self.rs('adcs_monitor.ssa_mode')

        self.rs("attitude_estimator.q_body_eci")
        self.rs("attitude_estimator.L_body")

        self.rs("adcs_cmd.mtr_cmd")
        self.rs("adcs_cmd.rwa_torque_cmd")
        self.rs("attitude_estimator.valid"),
        self.rs("attitude_estimator.w_bias_body"),
        self.rs("orbit.valid")
        

    def run(self):
        """
        Log/compute all the things necessary to monitor performance
        """

        """
        The below call is necessary to step FSW, simulate psim,
        as well as call any autotelem/dbtelem
        """
        self.cycle()

        self.rs_psim("truth.leader.attitude.w")
        self.rs_psim("truth.t.ns")
        self.rs_psim("truth.dt.ns")

        self.data_logs()
