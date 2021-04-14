from ...data_consumers import Logger
import time
import math
import threading
import traceback
from ..utils import Enums, TestCaseFailure, suppress_faults
import psim # the actual python psim repo
import lin
import datetime
from .ptest_case import PTestCase

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
        self.initial_state_timeout = 25
        self.leader_skip_deployment_wait = False
        self.follower_skip_deployment_wait = False
        self.leader_suppress_faults = True
        self.follower_suppress_faults = True

    def populate_devices(self, devices, radios):
        if devices:
            self.flight_controller_leader = devices["FlightControllerLeader"]
            self.flight_controller_follower = devices["FlightControllerFollower"]
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
            self.logger.put("[TESTCASE] Suppressing leader faults!")
            suppress_faults(self.flight_controller_leader, self.logger)

        if self.follower_suppress_faults:
            self.logger.put("[TESTCASE] Suppressing follower faults!")
            suppress_faults(self.flight_controller_follower, self.logger)

        if self.leader_skip_deployment_wait:
            self.logger.put("[TESTCASE] Skipping leader deployment wait!")
            self.flight_controller_leader.write_state("pan.deployment.elapsed", "15000")

        if self.follower_skip_deployment_wait:
            self.logger.put("[TESTCASE] Skipping follower deployment wait!")
            self.flight_controller_follower.write_state("pan.deployment.elapsed", "15000")

        self.pre_boot()

        cycles = 0
        leader_initial_state = Enums.mission_states[self.leader_initial_state]
        leader_state = self.flight_controller_leader.smart_read("pan.state")
        follower_initial_state = Enums.mission_states[self.follower_initial_state]
        follower_state = self.flight_controller_follower.smart_read("pan.state")
        self.logger.put("[TESTCASE] Boot Util waiting to reach initial states")
        while leader_state != leader_initial_state or follower_state != follower_initial_state:
            if cycles > self.initial_state_timeout:
                raise TestCaseFailure(f"Failed to reach desired states of {leader_initial_state} and {follower_initial_state}, was {leader_state} and {follower_state}")

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
            if device.scrape:
                device.scrape_uplink()
            if device.enable_auto_dbtelem:
                device.dbtelem()

    @property
    def mission_state_leader(self):
        return Enums.mission_states[int(self.flight_controller_leader.read_state("pan.state"))]

    @property
    def mission_state_follower(self):
        return Enums.mission_states[int(self.flight_controller_follower.read_state("pan.state"))]

    @mission_state_leader.setter
    def mission_state_leader(self, state):
        self.flight_controller_leader.write_state("pan.state", int(Enums.mission_states[state]))

    @mission_state_follower.setter
    def mission_state_follower(self, state):
        self.flight_controller_follower.write_state("pan.state", int(Enums.mission_states[state]))

    def read_state_leader(self, string_state):
        return self.flight_controller_leader.read_state(string_state)

    def write_state_leader(self, string_state, state_value):
        self.flight_controller_leader.write_state(string_state, state_value)
        return self.flight_controller_leader.read_state(string_state)

    def read_state_follower(self, string_state):
        return self.flight_controller_follower.read_state(string_state)

    def write_state_follower(self, string_state, state_value):
        self.flight_controller_follower.write_state(string_state, state_value)
        return self.flight_controller_follower.read_state(string_state)