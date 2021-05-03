from ...gpstime import GPSTime
from ..utils import Enums, TestCaseFailure
from .standby import DualSatStandbyCase
from .utils import log_fc_data, log_psim_data

import lin


class DualSatFarFieldCase(DualSatStandbyCase):

    def __init__(self, *args, **kwargs):
        super(DualSatFarFieldCase, self).__init__(*args, **kwargs)

        self.psim_config_override.update({
            "truth.t.ns": 5695000000000,
            "truth.leader.orbit.r": lin.Vector3([6.36085e+06, -2.53567e+06, 288872]),
            "truth.leader.orbit.v": lin.Vector3([1603.48, 4636.37, 5385.76]),
            "truth.follower.orbit.r": lin.Vector3([6.36143e+06, -2.53408e+06, 290549]),
            "truth.follower.orbit.v": lin.Vector3([1601.09, 4637.21, 5385.54])
        })


    def post_boot(self):
        """To put us in fair field, we need to be able to initialize each
        satellite's relative orbit estimate. The purpose of this post boot
        function is to simulate orbital uplinks from the ground and initialize
        their relative orbit estimates.
        """
        from psim import Simulation, Configuration
        from psim.sims import SingleOrbitGnc

        import lin

        self.logger.put("[TESTCASE] Initializing the relative orbit estimates.")

        configs = ["truth/base", "sensors/base"]
        configs = [
            self.psim_config_prefix + config + self.psim_config_suffix
            for config in configs
        ]
        config = Configuration(configs)

        def propagate(t, r, v, steps):
            """Propagate and orbital state forward in time by the requested
            number of steps.
            """
            config["truth.t.ns"] = GPSTime(*t).to_pan_ns()
            config["truth.leader.orbit.r"] = lin.Vector3(r)
            config["truth.leader.orbit.v"] = lin.Vector3(v)

            sim = Simulation(SingleOrbitGnc, config)
            for _ in range(steps):
                sim.step()

            return GPSTime(sim["truth.t.ns"]).to_list(), \
                   list(sim["truth.leader.orbit.r"]), \
                   list(sim["truth.leader.orbit.v"]),

        def uplink(satellite, other):
            """Propagate this satellites orbital state forward in time and
            uplink it to the other spacecraft.
            """
            t, r, v = propagate(
                satellite.smart_read("time.gps"),
                satellite.smart_read("orbit.pos"),
                satellite.smart_read("orbit.vel"),
                10
            )

            other.write_state("rel_orbit.uplink.time", t)
            other.write_state("rel_orbit.uplink.pos", r)
            other.write_state("rel_orbit.uplink.vel", v)

        uplink(self.flight_controller_leader, self.flight_controller_follower)
        uplink(self.flight_controller_follower, self.flight_controller_leader)

        self.logger.put("[TESTCASE] Setting leader and follower sat designations.")

        self.flight_controller_leader.write_state("pan.sat_designation", Enums.sat_designations["leader"])
        self.flight_controller_follower.write_state("pan.sat_designation", Enums.sat_designations["follower"])

        for _ in range(15):
            self.cycle()

        if Enums.rel_orbit_state[self.flight_controller_leader.smart_read("rel_orbit.state")] != "propagating":
            raise TestCaseFailure("The leader's relative orbit estimator should be propagating.")
        if Enums.mission_states[self.flight_controller_leader.smart_read("pan.state")] != "leader":
            raise TestCaseFailure("The leader's mission state should be in leader.")

        if Enums.rel_orbit_state[self.flight_controller_follower.smart_read("rel_orbit.state")] != "propagating":
            raise TestCaseFailure("The follower's relative orbit estimator should be propagating.")
        if Enums.mission_states[self.flight_controller_follower.smart_read("pan.state")] != "follower":
            raise TestCaseFailure("The follower's mission state should be in follower.")

    def run(self):
        self.cycle()

        if not self.is_interactive:
            self.finish()
            return

        log_fc_data(self.flight_controller_leader)
        log_fc_data(self.flight_controller_follower)
        log_psim_data(self, "leader", "follower")
