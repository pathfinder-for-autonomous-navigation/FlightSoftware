from ..base import DualSatCase, PSimCase
from ..utils import Enums, TestCaseFailure
from .utils import log_fc_data, log_psim_data


class DualSatNearFieldCase(DualSatCase, PSimCase):

    def __init__(self, *args, **kwargs):
        super(DualSatNearFieldCase, self).__init__(*args, **kwargs)

        self.psim_configs += ["truth/near_field"]
        self.leader_initial_state = "standby"
        self.follower_initial_state = "standby"
        self.leader_skip_deployment_wait = True
        self.follower_skip_deployment_wait = True
        self.allow_rtk = False

    def post_boot(self):
        pass

        # """To put us in fair field, we need to be able to initialize each
        # satellite's relative orbit estimate. The purpose of this post boot
        # function is to simulate orbital uplinks from the ground and initialize
        # their relative orbit estimates.
        # """
        # self.logger.put("[TESTCASE] Setting leader and follower sat designations.")

        # self.flight_controller_leader.write_state("pan.sat_designation", Enums.sat_designations["leader"])
        # self.flight_controller_follower.write_state("pan.sat_designation", Enums.sat_designations["follower"])

        # self.cycle()

        # if not self.rs_psim("sensors.leader.cdgps.valid"):
        #     raise TestCaseFailure("Leader CDGPS sensors should be valid.")
        # if not self.rs_psim("sensors.follower.cdgps.valid"):
        #     raise TestCaseFailure("Follower CDGPS sensors should be valid.")

        # for _ in range(10):
        #     self.cycle()

        # if Enums.rel_orbit_state[self.flight_controller_leader.smart_read("rel_orbit.state")] != "estimating":
        #     raise TestCaseFailure("The leader's relative orbit estimator should be propagating.")
        # if Enums.mission_states[self.flight_controller_leader.smart_read("pan.state")] != "leader_close_approach":
        #     raise TestCaseFailure("The leader's mission state should be in leader close approach.")

        # if Enums.rel_orbit_state[self.flight_controller_follower.smart_read("rel_orbit.state")] != "estimating":
        #     raise TestCaseFailure("The follower's relative orbit estimator should be propagating.")
        # if Enums.mission_states[self.flight_controller_follower.smart_read("pan.state")] != "follower_close_approach":
        #     raise TestCaseFailure("The follower's mission state should be in follower close approach.")

    def run(self):
        
        # disable RTK data forwarding.
        
        # satellites are in standby initially
        
        # ptest write them to leader and follower
        
        # have AMC running in the background
        
        # after 30s, expecting AMC to have relayed position, time, velocity data such that
        
        # assert that rel_orbit.state == 1, on both satellites
        
        # allow rtk = true
        
        # assert that rel_orbit.state == 2, on both satellites
        
        # assert pan state == close approaches
        
        self.cycle()

        if not self.is_interactive:
            self.finish()
            return

        log_fc_data(self.flight_controller_leader)
        log_fc_data(self.flight_controller_follower)
        log_psim_data(self, "leader", "follower")
