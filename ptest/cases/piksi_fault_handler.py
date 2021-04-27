from .base import SingleSatCase, PSimCase
from .utils import FSWEnum, Enums, TestCaseFailure

class PiksiFaultHandler(SingleSatCase, PSimCase):
    def __init__(self, *args, **kwargs):
        super(PiksiFaultHandler, self).__init__(*args, **kwargs)
        self.initial_state = "standby"
        self.psim_configs += ["truth/standby"]
        self.debug_to_console = True
        self.skip_deployment_wait = True

    def post_boot(self):
        self.mission_state = "leader"
        self.mock_sensor_validity = True
        self.ws("fault_handler.enabled", True)
        self.ws("piksi_fh.enabled", True)
        ####### Had to comment out piski_fh.dead.suppress in utils.py to run #######
        self.ws("piski_fh.dead.suppress", False)
        self.cycle()
        self.collect_diagnostic_data()

    def collect_diagnostic_data(self):
        self.rs("piksi.state")
        self.rs("pan.state")

    def check_is_leader(self):
        if self.mission_state != "leader":
            raise TestCaseFailure("Failed to set mission state to leader. Mission state is " + self.mission_state + ".")

    def check_is_standby(self):
        if self.mission_state != "standby":
            raise TestCaseFailure("Failed to set mission state to 'standby'.")

    def check_piksi_dead_fault(self, error="crc_error", status=None):
        self.mission_state = "leader"
        self.cycle()
        self.collect_diagnostic_data()

        # Cycle for time until fault would be triggered
        for _ in range(self.one_day_ccno//6 + 1):
            self.check_is_leader()
            self.ws("piksi.state",Enums.piksi_modes[error])

            self.cycle()
            self.collect_diagnostic_data()

        if status=="should_fault":
            self.check_is_standby()
        else:
            if self.mission_state == "standby":
                raise TestCaseFailure("Piksi dead fault incorrectly set mission state to 'standby'.")

    def run(self):
        # Test 1: Set piksi mode to 'dead'
        self.logger.put("Running first test: Piksi is Dead")

        self.check_piksi_dead_fault(error="crc_error", status="should_fault")
        self.check_piksi_dead_fault(error="no_data_error", status="should_fault")
        self.check_piksi_dead_fault(error="data_error", status="should_fault")

        self.logger.put("PiksiFaultHandler successfully changed mission state to 'standby' when piksi mode was dead.")
        self.finish()