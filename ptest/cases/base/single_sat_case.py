from ...data_consumers import Logger
from ..utils import Enums, TestCaseFailure, suppress_faults
from .ptest_case import FancyFlightController, PTestCase


class SingleSatCase(PTestCase):
    """Base class for all HOOTL and HITL testcases involving a single satellite.

    Attributes:
        initial_state  Set this to the string name of a desired flight software
                       state. The testcase will cycle in setup until this state
                       is reached. Defaults to 'startup'.

        initial_state_timeout  Timeout period in cycles in which flight software
                               has to get into the desired initial state. If
                               this timeout is violated, the testcase will fail.

        skip_deployment_wait  Set this to true to skip the deployment wait. This
                              sets the deployment elapsed counter high enough in
                              setup so the satellite skips deployment wait on
                              its very next cycle. Defaults to false.

        suppress_faults  Set this to true to suppress faults in setup. Defaults
                         to true.
    """
    def __init__(self, *args, **kwargs):
        super(SingleSatCase, self).__init__(*args, **kwargs)

        self.initial_state = "startup"
        self.initial_state_timeout = 100
        self.skip_deployment_wait = False
        self.suppress_faults = True

    def populate_devices(self, devices, radios):
        self.flight_controller = devices["FlightController"]
        self.leader = FancyFlightController(self.flight_controller, self.logger)
        self.devices = [self.flight_controller]

    def setup(self, devices, radios):
        super(SingleSatCase, self).setup(devices, radios)

        if self.suppress_faults:
            self.logger.put("[TESTCASE] Suppressing faults.")
            suppress_faults(self.flight_controller, self.logger)

        if self.skip_deployment_wait:
            self.logger.put("[TESTCASE] Skipping deployment wait.")
            self.flight_controller.write_state("pan.deployment.elapsed", "15000")

        if self.flight_controller.enable_auto_dbtelem:
            self.logger.put("[TESTCASE] Auto-dbtelem ACTIVE")
        else:
            self.logger.put("[TESTCASE] Auto-dbtelem INACTIVE")

        if self.flight_controller.scrape_uplink:
            self.logger.put("[TESTCASE] Scrape-uplink ACTIVE")
        else:
            self.logger.put("[TESTCASE] Scrape-uplink INACTIVE")

        self.pre_boot()

        cycles = 0
        initial_state = Enums.mission_states[self.initial_state]
        state = self.flight_controller.smart_read("pan.state")
        self.logger.put("[TESTCASE] Boot utility waiting to reach initial state.")
        while state != initial_state:
            
            if cycles > self.initial_state_timeout:
                raise TestCaseFailure(f"Failed to reach desired state of {initial_state}, was {state}.")

            self.cycle()

            cycles = cycles + 1
            state = self.flight_controller.smart_read("pan.state")

        self.one_day_ccno = self.flight_controller.smart_read("pan.one_day_ccno")
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

    @property
    def mission_state(self):
        return self.leader.mission_state

    @mission_state.setter
    def mission_state(self, state):
        self.leader.mission_state = state

    def cycle(self):
        """
        Steps the FC forward by one CC

        Asserts the FC did indeed step forward by one CC
        """
        self.ws('cycle.start', True)

        super(SingleSatCase, self).cycle()

        if self.flight_controller.scrape:
            self.flight_controller.scrape_uplink()
        if self.flight_controller.enable_auto_dbtelem:
            self.flight_controller.dbtelem()

    def rs(self, name, print=False):
        return self.leader.rs(name, print=print)

    def print_rs(self, name):
        return self.leader.rs(name, print=True)

    def ws(self, name, val, print=False):
        self.leader.ws(name, val, print=print)

    def print_ws(self, name, val):
        self.leader.ws(name, val, print=True)
