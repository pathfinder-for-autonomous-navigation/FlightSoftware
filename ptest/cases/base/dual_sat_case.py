from ...data_consumers import Logger
from ..utils import Enums, TestCaseFailure, suppress_faults
from .ptest_case import FancyFlightController, PTestCase


class DualSatCase(PTestCase):
    """Base class for all HOOTL and HITL testcases involving two satellites.
    See the SingleSatCase for descriptions of the attributes. Many of the other
    functions used here mirror elements of SingleSatCase as well so be sure to
    checkout the documentation there too.
    """
    def __init__(self, *args, **kwargs):
        super(DualSatCase, self).__init__(*args, **kwargs)

        self.leader_initial_state = "startup"
        self.follower_initial_state = "startup"
        self.initial_state_timeout = 100
        self.leader_skip_deployment_wait = True
        self.follower_skip_deployment_wait = True
        self.leader_suppress_faults = True
        self.follower_suppress_faults = True
        self.check_initial_state = True

    def populate_devices(self, devices, radios):
        if devices:
            self.flight_controller_leader = devices["FlightControllerLeader"]
            self.leader = FancyFlightController(self.flight_controller_leader, self.logger)
            self.flight_controller_follower = devices["FlightControllerFollower"]
            self.follower = FancyFlightController(self.flight_controller_follower, self.logger)
            self.devices = [self.flight_controller_leader, self.flight_controller_follower]
        if "FlightControllerLeaderRadio" in radios:
            self.radio_leader = radios["FlightControllerLeaderRadio"]
            self.radio_follower = radios["FlightControllerFollowerRadio"]
        else:
            self.radio_leader = None
            self.radio_follower = None

    def setup(self, devices, radios):
        super(DualSatCase, self).setup(devices, radios)

        if self.leader_suppress_faults:
            self.logger.put("[TESTCASE] Suppressing leader faults.")
            suppress_faults(self.flight_controller_leader, self.logger)

        if self.follower_suppress_faults:
            self.logger.put("[TESTCASE] Suppressing follower faults.")
            suppress_faults(self.flight_controller_follower, self.logger)

        if self.flight_controller_leader.enable_auto_dbtelem:
            self.logger.put("[TESTCASE] Leader: Auto-dbtelem ACTIVE")
        else:
            self.logger.put("[TESTCASE] Leader: Auto-dbtelem INACTIVE")

        if self.flight_controller_follower.enable_auto_dbtelem:
            self.logger.put("[TESTCASE] Follower: Auto-dbtelem ACTIVE")
        else:
            self.logger.put("[TESTCASE] Follower: Auto-dbtelem INACTIVE")

        if self.flight_controller_leader.scrape:
            self.logger.put("[TESTCASE] Leader: Scrape-uplink ACTIVE")
        else:
            self.logger.put("[TESTCASE] Leader: Scrape-uplink INACTIVE")

        if self.flight_controller_follower.scrape:
            self.logger.put("[TESTCASE] Follower: Scrape-uplink ACTIVE")
        else:
            self.logger.put("[TESTCASE] Follower: Scrape-uplink INACTIVE")

        if self.leader_skip_deployment_wait:
            self.logger.put("[TESTCASE] Skipping leader deployment wait.")
            self.flight_controller_leader.write_state("pan.deployment.elapsed", "106")

        if self.follower_skip_deployment_wait:
            self.logger.put("[TESTCASE] Skipping follower deployment wait.")
            self.flight_controller_follower.write_state("pan.deployment.elapsed", "106")

        self.pre_boot()

        cycles = 0
        leader_initial_state = Enums.mission_states[self.leader_initial_state]
        leader_state = self.flight_controller_leader.smart_read("pan.state")
        follower_initial_state = Enums.mission_states[self.follower_initial_state]
        follower_state = self.flight_controller_follower.smart_read("pan.state")
        self.logger.put("[TESTCASE] Boot utility waiting to reach initial states.")
        while (leader_state != leader_initial_state or follower_state != follower_initial_state) and self.check_initial_state:
            if cycles > self.initial_state_timeout:
                raise TestCaseFailure(f"Failed to reach desired states of {leader_initial_state} and {follower_initial_state}, was {leader_state} and {follower_state}.")

            self.cycle()

            cycles = cycles + 1
            leader_state = self.flight_controller_leader.smart_read("pan.state")
            follower_state = self.flight_controller_follower.smart_read("pan.state")

        self.leader_one_day_ccno = self.flight_controller_leader.smart_read("pan.one_day_ccno")
        self.follower_one_day_ccno = self.flight_controller_follower.smart_read("pan.one_day_ccno")
        self.post_boot()

    def pre_boot(self):
        """Allows a testcase to perform actions just after supressing faults and
        skipping deployment wait but before attempting to boot the spacecraft.
        See setup for more information.
        """
        pass

    def post_boot(self):
        """Allows a testcase to perform actions just after booting the
        spacecraft. See setup for more information.
        """
        pass

    def cycle(self):
        """Step the flight controllers forward by one CC
        """
        self.flight_controller_leader.write_state('cycle.start', 'true')
        self.flight_controller_follower.write_state('cycle.start', 'true')

        super(DualSatCase, self).cycle()

        for device in self.devices:
            #if device.scrape:
            #    device.scrape_uplink()
            if device.enable_auto_dbtelem:
                device.dbtelem()
